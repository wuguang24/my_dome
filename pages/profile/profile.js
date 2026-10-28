// pages/profile/profile.js
const app = getApp();

Page({
  data: {
    userInfo: null,
    isLoggedIn: false,
    showPasswordModal: false,
    oldPassword: '',
    newPassword: '',
    confirmPassword: ''
  },

  onLoad() {
    this.checkLoginStatus();
  },

  onShow() {
    this.checkLoginStatus();
  },

  checkLoginStatus() {
    this.setData({
      userInfo: app.globalData.userInfo,
      isLoggedIn: app.globalData.isLoggedIn
    });
  },

  // 跳转登录
  goLogin() {
    wx.navigateTo({ url: '/pages/login/login' });
  },

  // 退出登录
  handleLogout() {
    wx.showModal({
      title: '确认退出',
      content: '确定要退出登录吗？',
      success: (res) => {
        if (res.confirm) {
          app.logout();
        }
      }
    });
  },

  // 显示修改密码弹窗
  showChangePassword() {
    this.setData({ 
      showPasswordModal: true,
      oldPassword: '',
      newPassword: '',
      confirmPassword: ''
    });
  },

  hidePasswordModal() {
    this.setData({ showPasswordModal: false });
  },

  onOldPasswordInput(e) {
    this.setData({ oldPassword: e.detail.value });
  },

  onNewPasswordInput(e) {
    this.setData({ newPassword: e.detail.value });
  },

  onConfirmPasswordInput(e) {
    this.setData({ confirmPassword: e.detail.value });
  },

  // 验证密码格式：8位数字或字母
  validatePassword(password) {
    const reg = /^[a-zA-Z0-9]{8}$/;
    return reg.test(password);
  },

  // 修改密码 - 通过属性设置
  async handleChangePassword() {
    const { oldPassword, newPassword, confirmPassword } = this.data;
    const username = this.data.userInfo?.username;

    if (!oldPassword || !newPassword || !confirmPassword) {
      wx.showToast({ title: '请填写完整', icon: 'none' });
      return;
    }

    if (newPassword !== confirmPassword) {
      wx.showToast({ title: '两次密码不一致', icon: 'none' });
      return;
    }

    if (!this.validatePassword(newPassword)) {
      wx.showToast({ title: '密码必须是8位数字或字母', icon: 'none' });
      return;
    }

    wx.showLoading({ title: '修改中...' });

    try {
      const config = {
        productID: app.globalData.productID,
        deviceID: app.globalData.deviceID,
        authorization: app.globalData.authorization
      };
      
      // 先获取当前账号数据
      const queryUrl = `${app.globalData.serverUrl}/thingmodel/query-device-property?product_id=${config.productID}&device_name=${config.deviceID}`;
      
      const queryRes = await new Promise((resolve, reject) => {
        wx.request({
          url: queryUrl,
          method: 'GET',
          header: { 'authorization': config.authorization },
          timeout: 10000,
          success: resolve,
          fail: reject
        });
      });

      if (queryRes.data.code !== 0) {
        throw new Error('获取数据失败');
      }

      const properties = queryRes.data.data || [];
      const accountProp = properties.find(p => p.identifier === 'account');
      const passwordProp = properties.find(p => p.identifier === 'password');
      
      let accounts = accountProp?.value ? accountProp.value.split(',') : [];
      let passwords = passwordProp?.value ? passwordProp.value.split(',') : [];
      
      // 合并本地缓存的账号（可能有刚注册的）
      const localAccounts = wx.getStorageSync('localAccounts') || {};
      for (const acc in localAccounts) {
        if (!accounts.includes(acc)) {
          accounts.push(acc);
          passwords.push(localAccounts[acc]);
        }
      }
      
      // 查找用户并验证旧密码
      const userIndex = accounts.findIndex(acc => acc.trim() === username);
      if (userIndex < 0) {
        wx.hideLoading();
        wx.showToast({ title: '用户不存在', icon: 'none' });
        return;
      }
      
      if (passwords[userIndex]?.trim() !== oldPassword) {
        wx.hideLoading();
        wx.showToast({ title: '原密码错误', icon: 'none' });
        return;
      }
      
      // 更新密码
      passwords[userIndex] = newPassword;
      
      // 同时更新本地缓存
      localAccounts[username] = newPassword;
      wx.setStorageSync('localAccounts', localAccounts);
      
      // 设置新密码
      const setUrl = `${app.globalData.serverUrl}/thingmodel/set-device-property`;
      
      const setRes = await new Promise((resolve, reject) => {
        wx.request({
          url: setUrl,
          method: 'POST',
          header: {
            'authorization': config.authorization,
            'Content-Type': 'application/json'
          },
          timeout: 30000,
          data: {
            product_id: config.productID,
            device_name: config.deviceID,
            params: {
              account: accounts.join(','),
              password: passwords.join(',')
            }
          },
          success: resolve,
          fail: reject
        });
      });

      wx.hideLoading();
      
      if (setRes.data?.code === 0) {
        wx.showToast({ title: '修改成功', icon: 'success' });
        this.hidePasswordModal();
      } else {
        wx.showToast({ title: setRes.data?.msg || '修改失败', icon: 'none' });
      }
    } catch (err) {
      wx.hideLoading();
      console.error('修改密码失败:', err);
      wx.showToast({ title: '网络错误', icon: 'none' });
    }
  },

  // 查看借还记录
  goRecords() {
    wx.switchTab({ url: '/pages/records/records' });
  },

  // 关于系统
  showAbout() {
    wx.showModal({
      title: '关于系统',
      content: '智能工具柜管理系统 v1.0\n\n基于ESP32+OneNet物联网平台',
      showCancel: false
    });
  }
})
