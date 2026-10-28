// pages/detail/detail.js
const app = getApp();

Page({
  data: {
    tool: null,
    loading: false,
    reserveTime: '',
    reserveDuration: 1
  },

  onLoad(options) {
    console.log('详情页加载, options:', options);
    const toolIndex = parseInt(options.index);
    const tools = app.globalData.tools;
    const tool = tools.find(t => t.index === toolIndex);
    
    console.log('找到的工具:', tool);
    
    if (tool) {
      this.setData({ tool });
      console.log('设置tool数据完成');
      this.fetchToolDetail(toolIndex);
    } else {
      console.error('未找到工具，toolIndex:', toolIndex);
    }
  },

  async fetchToolDetail(toolIndex) {
    console.log('开始获取工具详情, toolIndex:', toolIndex);
    // 获取工具详细状态
    const config = this.getConfig();
    const url = `${app.globalData.serverUrl}/thingmodel/query-device-property?product_id=${config.productID}&device_name=${config.deviceID}`;
    
    wx.request({
      url,
      method: 'GET',
      header: { 'authorization': config.authorization },
      success: (res) => {
        console.log('获取工具状态响应:', res.data);
        if (res.data.code === 0) {
          const properties = res.data.data || [];
          const toolStatusProp = properties.find(p => p.identifier === 'tool_status');
          const bitmap = toolStatusProp?.value || 0;
          const isBorrowed = (bitmap >> (toolIndex - 1)) & 1;
          
          console.log('工具状态 - bitmap:', bitmap, 'isBorrowed:', isBorrowed);
          
          this.setData({
            'tool.isBorrowed': isBorrowed,
            'tool.statusText': isBorrowed ? '使用中' : '空闲',
            'tool.statusColor': isBorrowed ? '#ff4d4f' : '#52c41a'
          });
          
          console.log('更新后的tool数据:', this.data.tool);
        }
      },
      fail: (err) => {
        console.error('获取工具状态失败:', err);
      }
    });
  },

  getConfig() {
    return {
      productID: app.globalData.productID,
      deviceID: app.globalData.deviceID,
      authorization: app.globalData.authorization
    };
  },

  // 借用工具
  async handleBorrow() {
    console.log('========== 借用按钮被点击 ==========');
    
    if (!app.checkLogin()) {
      console.log('用户未登录');
      return;
    }
    
    console.log('开始借用流程，工具:', this.data.tool);
    
    this.setData({ loading: true });
    wx.showLoading({ title: '开锁中...' });

    try {
      const config = this.getConfig();
      
      // OneNet物模型服务调用API
      const url = `${app.globalData.serverUrl}/thingmodel/call-service`;
      
      console.log('=== 开始调用服务 ===');
      console.log('API地址:', url);
      console.log('产品ID:', config.productID);
      console.log('设备名称:', config.deviceID);
      console.log('Authorization:', config.authorization);
      
      await this.callService(url, config, 'RemoteUnlock', {
        ToolIndex: this.data.tool.index,
        Duration: 30,
        Mode: 1  // 1=借用模式
      });

      wx.hideLoading();
      wx.showModal({
        title: '已开锁',
        content: '请打开柜门，取出工具后扫描RFID确认',
        showCancel: false,
        success: () => {
          setTimeout(() => {
            this.fetchToolDetail(this.data.tool.index);
          }, 5000);
        }
      });

    } catch (err) {
      wx.hideLoading();
      console.error('=== 服务调用失败 ===');
      console.error('错误详情:', err);
      
      // 构建详细的错误信息
      let errorMsg = '操作失败\n\n';
      
      if (err?.data) {
        errorMsg += `返回数据: ${JSON.stringify(err.data)}\n`;
      }
      if (err?.msg) {
        errorMsg += `错误消息: ${err.msg}\n`;
      }
      if (err?.code) {
        errorMsg += `错误码: ${err.code}\n`;
      }
      if (err?.errno) {
        errorMsg += `网络错误: ${err.errno}\n`;
      }
      if (err?.errMsg) {
        errorMsg += `错误信息: ${err.errMsg}\n`;
      }
      
      // 如果没有任何错误信息，显示完整对象
      if (!err?.msg && !err?.code && !err?.errno && !err?.errMsg) {
        errorMsg += `完整错误: ${JSON.stringify(err)}`;
      }
      
      wx.showModal({
        title: '服务调用失败',
        content: errorMsg,
        showCancel: true,
        cancelText: '关闭',
        confirmText: '复制错误',
        success: (res) => {
          if (res.confirm) {
            wx.setClipboardData({
              data: errorMsg,
              success: () => {
                wx.showToast({ title: '已复制到剪贴板', icon: 'success' });
              }
            });
          }
        }
      });
    } finally {
      this.setData({ loading: false });
    }
  },

  // 归还工具
  async handleReturn() {
    this.setData({ loading: true });
    wx.showLoading({ title: '开锁中...' });

    try {
      const config = this.getConfig();
      const url = `${app.globalData.serverUrl}/thingmodel/call-service`;

      console.log('=== 开始调用归还服务 ===');
      
      await this.callService(url, config, 'RemoteUnlock', {
        ToolIndex: this.data.tool.index,
        Duration: 30,
        Mode: 0  // 0=归还模式
      });

      wx.hideLoading();
      wx.showModal({
        title: '已开锁',
        content: '请打开柜门，放入工具后扫描RFID确认归还',
        showCancel: false,
        success: () => {
          setTimeout(() => {
            this.fetchToolDetail(this.data.tool.index);
          }, 5000);
        }
      });

    } catch (err) {
      wx.hideLoading();
      console.error('=== 归还服务调用失败 ===');
      console.error('错误详情:', err);
      
      let errorMsg = '操作失败';
      if (err?.msg) {
        errorMsg = err.msg;
      } else if (err?.errno) {
        errorMsg = `网络错误: ${err.errno}`;
      } else if (err?.code) {
        errorMsg = `错误码: ${err.code}`;
      }
      
      wx.showModal({
        title: '操作失败',
        content: errorMsg,
        showCancel: false
      });
    } finally {
      this.setData({ loading: false });
    }
  },

  callService(url, config, identifier, params) {
    return new Promise((resolve, reject) => {
      wx.request({
        url,
        method: 'POST',
        header: {
          'authorization': config.authorization,
          'Content-Type': 'application/json'
        },
        timeout: 10000,
        data: {
          product_id: config.productID,
          device_name: config.deviceID,
          identifier,
          params
        },
        success: (res) => {
          console.log('服务响应:', res);
          if (res.data?.code === 0) {
            resolve(res.data);
          } else {
            console.error('服务调用失败:', res.data);
            reject(res.data);
          }
        },
        fail: (err) => {
          console.error('请求失败:', err);
          reject(err);
        }
      });
    });
  },

  saveBorrowRecord() {
    const records = wx.getStorageSync('records') || [];
    records.unshift({
      type: 'borrow',
      toolIndex: this.data.tool.index,
      toolName: this.data.tool.name,
      user: app.globalData.userInfo?.username || '未知用户',
      time: new Date().toLocaleString(),
      timestamp: Date.now()
    });
    wx.setStorageSync('records', records.slice(0, 100));
  },

  saveReturnRecord() {
    const records = wx.getStorageSync('records') || [];
    records.unshift({
      type: 'return',
      toolIndex: this.data.tool.index,
      toolName: this.data.tool.name,
      user: app.globalData.userInfo?.username || '未知用户',
      time: new Date().toLocaleString(),
      timestamp: Date.now()
    });
    wx.setStorageSync('records', records.slice(0, 100));
  },

  // 预约工具
  handleReserve() {
    wx.showToast({ title: '预约功能开发中', icon: 'none' });
  }
})
