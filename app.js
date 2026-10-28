// app.js
App({
  onLaunch() {
    // 检查登录状态
    const userInfo = wx.getStorageSync('userInfo');
    if (userInfo) {
      this.globalData.userInfo = userInfo;
      this.globalData.isLoggedIn = true;
    }
  },

  globalData: {
    // OneNet 配置
    serverUrl: "https://iot-api.heclouds.com",
    productID: "40PeGZrCs7",
    deviceID: "TOOLBOX_SINGLE",
    authorization: "version=2018-10-31&res=products%2F40PeGZrCs7%2Fdevices%2FTOOLBOX_SINGLE&et=1778945770&method=sha1&sign=Xzlg5mt3PlyH0CsaMbVKZYTFvx4%3D",
    
    // 用户信息
    userInfo: null,
    isLoggedIn: false,
    
    // 工具列表
    tools: [
      { index: 1, name: '螺丝刀', icon: '🔧' },
      { index: 2, name: '钳子', icon: '🔨' },
      { index: 3, name: '扳手', icon: '🔩' },
      { index: 4, name: '锤子', icon: '⚒️' },
      { index: 5, name: '卷尺', icon: '📏' },
      { index: 6, name: '工具刀', icon: '🔪' }
    ]
  },

  // 检查登录状态
  checkLogin() {
    if (!this.globalData.isLoggedIn) {
      wx.redirectTo({ url: '/pages/login/login' });
      return false;
    }
    return true;
  },

  // 登录
  login(userInfo) {
    this.globalData.userInfo = userInfo;
    this.globalData.isLoggedIn = true;
    wx.setStorageSync('userInfo', userInfo);
  },

  // 登出
  logout() {
    this.globalData.userInfo = null;
    this.globalData.isLoggedIn = false;
    wx.removeStorageSync('userInfo');
    wx.redirectTo({ url: '/pages/login/login' });
  }
})
