// pages/index/index.js
const util = require('../../utils/util.js');
const app = getApp();

Page({
  data: {
    tools: [],
    loading: true,
    isLoggedIn: false
  },

  onLoad() {
    this.initTools();
    this.fetchToolStatus();
  },

  onShow() {
    this.setData({ isLoggedIn: app.globalData.isLoggedIn });
    
    // 启动定时刷新（页面显示时）
    if (!this.refreshTimer) {
      this.refreshTimer = setInterval(() => {
        if (!this.data.loading) {
          this.fetchToolStatusSilent();
        }
      }, 5000); // 5秒刷新一次
    }
  },

  onHide() {
    // 页面隐藏时停止定时器
    if (this.refreshTimer) {
      clearInterval(this.refreshTimer);
      this.refreshTimer = null;
    }
  },

  onUnload() {
    if (this.refreshTimer) {
      clearInterval(this.refreshTimer);
      this.refreshTimer = null;
    }
  },

  initTools() {
    const tools = app.globalData.tools.map(t => ({
      ...t,
      isBorrowed: false,
      statusText: '加载中',
      statusColor: '#999',
      loading: false
    }));
    this.setData({ tools });
  },

  goDetail(e) {
    const index = e.currentTarget.dataset.index;
    wx.navigateTo({ url: `/pages/detail/detail?index=${index}` });
  },

  getOneNetConfig() {
    const { productID, deviceID, authorization } = app.globalData;
    if (!productID || !deviceID || !authorization) {
      throw new Error('OneNet configuration error.');
    }
    return { productID, deviceID, authorization };
  },

  fetchToolStatusSilent() {
    try {
      const config = this.getOneNetConfig();
      const url = `${app.globalData.serverUrl}/thingmodel/query-device-property?product_id=${config.productID}&device_name=${config.deviceID}`;
      
      wx.request({
        url,
        method: 'GET',
        header: { 'authorization': config.authorization },
        timeout: 5000, // 添加超时
        success: (res) => {
          if (res.statusCode === 200 && res.data.code === 0) {
            this.updateToolsFromResponse(res.data.data);
          }
        }
      });
    } catch (err) {
      console.error('fetchToolStatusSilent error:', err);
    }
  },

  fetchToolStatus() {
    this.setData({ loading: true });
    
    try {
      const config = this.getOneNetConfig();
      const url = `${app.globalData.serverUrl}/thingmodel/query-device-property?product_id=${config.productID}&device_name=${config.deviceID}`;
      
      wx.request({
        url,
        method: 'GET',
        header: { 'authorization': config.authorization },
        timeout: 8000, // 添加超时
        success: (res) => {
          if (res.statusCode === 200 && res.data.code === 0) {
            this.updateToolsFromResponse(res.data.data);
          } else {
            wx.showToast({ title: '获取数据失败', icon: 'none' });
          }
          this.setData({ loading: false });
        },
        fail: () => {
          wx.showToast({ title: '网络错误', icon: 'none' });
          this.setData({ loading: false });
        }
      });
    } catch (err) {
      wx.showToast({ title: '配置错误', icon: 'none' });
      this.setData({ loading: false });
    }
  },

  updateToolsFromResponse(properties) {
    let bitmap = 0;
    if (Array.isArray(properties)) {
      const toolStatusProp = properties.find(item => item.identifier === 'tool_status');
      if (toolStatusProp) {
        bitmap = toolStatusProp.value || 0;
      }
    }
    const updatedTools = util.parseToolStatus(bitmap, app.globalData.tools);
    const toolsWithLoading = updatedTools.map(t => {
      const current = this.data.tools.find(ct => ct.index === t.index);
      return { ...t, loading: current?.loading || false };
    });
    this.setData({ tools: toolsWithLoading });
  },

  async handleBorrow(e) {
    const toolIndex = e.currentTarget.dataset.index;
    const isBorrowed = e.currentTarget.dataset.borrowed;
    
    console.log('首页 - 点击借用/归还按钮, toolIndex:', toolIndex, 'isBorrowed:', isBorrowed);
    
    // 检查登录
    if (!app.globalData.isLoggedIn) {
      wx.showModal({
        title: '提示',
        content: '请先登录',
        success: (res) => {
          if (res.confirm) {
            wx.navigateTo({ url: '/pages/login/login' });
          }
        }
      });
      return;
    }
    
    // 设置按钮加载状态
    const tools = this.data.tools.map(t => {
      if (t.index === toolIndex) return { ...t, loading: true };
      return t;
    });
    this.setData({ tools });
    
    wx.showLoading({ title: '开锁中...' });

    try {
      const config = this.getOneNetConfig();
      const url = `${app.globalData.serverUrl}/thingmodel/call-service`;
      
      console.log('首页 - 调用服务, identifier: RemoteUnlock');
      
      await this.callService(url, config, 'RemoteUnlock', {
        ToolIndex: toolIndex,
        Duration: 30,
        Mode: isBorrowed ? 0 : 1  // 已借出=归还(0), 未借出=借用(1)
      });

      wx.hideLoading();
      wx.showToast({ 
        title: '已开锁，请扫描RFID', 
        icon: 'none',
        duration: 3000
      });
      
      setTimeout(() => this.fetchToolStatus(), 5000);

    } catch (err) {
      wx.hideLoading();
      console.error('首页 - 服务调用失败:', err);
      wx.showToast({ title: err?.msg || '操作失败', icon: 'none' });
    } finally {
      const updatedTools = this.data.tools.map(t => {
        if (t.index === toolIndex) return { ...t, loading: false };
        return t;
      });
      this.setData({ tools: updatedTools });
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
          if (res.data?.code === 0) resolve(res.data);
          else reject(res.data);
        },
        fail: reject
      });
    });
  }
});
