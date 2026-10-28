// pages/login/login.js
const app = getApp();

Page({
  data: {
    isLogin: true,
    username: '',
    password: '',
    confirmPassword: '',
    loading: false,
    cloudAccounts: null  // 云平台账号数据
  },

  onLoad() {
    if (app.globalData.isLoggedIn) {
      wx.switchTab({ url: '/pages/index/index' });
      return;
    }
    // 从云平台同步账号数据
    this.syncAccountsFromCloud();
  },

  // 从云平台同步账号数据
  async syncAccountsFromCloud() {
    try {
      const config = {
        productID: app.globalData.productID,
        deviceID: app.globalData.deviceID,
        authorization: app.globalData.authorization
      };
      
      const url = `${app.globalData.serverUrl}/thingmodel/query-device-property?product_id=${config.productID}&device_name=${config.deviceID}`;
      
      wx.request({
        url,
        method: 'GET',
        header: { 'authorization': config.authorization },
        success: (res) => {
          if (res.data.code === 0 && res.data.data) {
            const properties = res.data.data;
            const accountProp = properties.find(p => p.identifier === 'account');
            const passwordProp = properties.find(p => p.identifier === 'password');
            
            if (accountProp && passwordProp) {
              const accounts = accountProp.value ? accountProp.value.split(',') : [];
              const passwords = passwordProp.value ? passwordProp.value.split(',') : [];
              
              const cloudAccounts = {};
              for (let i = 0; i < accounts.length; i++) {
                const acc = accounts[i].trim();
                const pwd = passwords[i] ? passwords[i].trim() : '';
                if (acc && pwd) {
                  cloudAccounts[acc] = pwd;
                }
              }
              
              // 合并本地缓存（本地可能有刚注册的账号）
              const localAccounts = wx.getStorageSync('localAccounts') || {};
              const mergedAccounts = { ...cloudAccounts, ...localAccounts };
              
              this.setData({ cloudAccounts: mergedAccounts });
              console.log('账号同步成功:', Object.keys(mergedAccounts));
            }
          }
        },
        fail: (err) => {
          console.error('同步账号失败:', err);
          // 失败时使用本地缓存
          const localAccounts = wx.getStorageSync('localAccounts') || {};
          if (Object.keys(localAccounts).length > 0) {
            this.setData({ cloudAccounts: localAccounts });
          }
        }
      });
    } catch (err) {
      console.error('syncAccountsFromCloud error:', err);
    }
  },

  switchMode() {
    this.setData({
      isLogin: !this.data.isLogin,
      username: '',
      password: '',
      confirmPassword: ''
    });
  },

  onUsernameInput(e) {
    this.setData({ username: e.detail.value });
  },

  onPasswordInput(e) {
    this.setData({ password: e.detail.value });
  },

  onConfirmPasswordInput(e) {
    this.setData({ confirmPassword: e.detail.value });
  },

  // 验证账号格式：8位数字
  validateUsername(username) {
    const reg = /^\d{8}$/;
    return reg.test(username);
  },

  // 验证密码格式：8位数字或字母
  validatePassword(password) {
    const reg = /^[a-zA-Z0-9]{8}$/;
    return reg.test(password);
  },

  // 登录 - 使用云平台账号验证
  async handleLogin() {
    const { username, password, cloudAccounts } = this.data;
    
    console.log('登录 - 输入账号:', username);
    console.log('登录 - 当前账号列表:', cloudAccounts);
    
    if (!username || !password) {
      wx.showToast({ title: '请填写完整信息', icon: 'none' });
      return;
    }

    // 验证账号格式
    if (!this.validateUsername(username)) {
      wx.showToast({ title: '账号必须是8位数字', icon: 'none' });
      return;
    }

    this.setData({ loading: true });

    // 优先使用云平台账号验证
    if (cloudAccounts && cloudAccounts[username]) {
      if (cloudAccounts[username] === password) {
        app.login({
          username,
          nickname: username,
          loginTime: new Date().toLocaleString()
        });
        
        wx.showToast({ title: '登录成功', icon: 'success' });
        setTimeout(() => {
          wx.switchTab({ url: '/pages/index/index' });
        }, 1000);
      } else {
        wx.showToast({ title: '密码错误', icon: 'none' });
      }
    } else {
      // 云平台没有此账号，提示注册
      console.log('账号不存在，cloudAccounts:', cloudAccounts);
      wx.showToast({ title: '账号不存在', icon: 'none' });
    }
    
    this.setData({ loading: false });
  },

  // 注册 - 通过云平台下发到ESP32存储
  async handleRegister() {
    const { username, password, confirmPassword, cloudAccounts } = this.data;
    
    if (!username || !password || !confirmPassword) {
      wx.showToast({ title: '请填写完整信息', icon: 'none' });
      return;
    }

    // 验证账号格式：8位数字
    if (!this.validateUsername(username)) {
      wx.showToast({ title: '账号必须是8位数字', icon: 'none' });
      return;
    }

    // 验证密码格式：8位数字或字母
    if (!this.validatePassword(password)) {
      wx.showToast({ title: '密码必须是8位数字或字母', icon: 'none' });
      return;
    }

    if (password !== confirmPassword) {
      wx.showToast({ title: '两次密码不一致', icon: 'none' });
      return;
    }

    // 检查用户名是否已存在
    if (cloudAccounts && cloudAccounts[username]) {
      wx.showToast({ title: '账号已存在', icon: 'none' });
      return;
    }

    this.setData({ loading: true });
    wx.showLoading({ title: '注册中...' });

    try {
      // 构建新的账号密码字符串
      let accounts = [];
      let passwords = [];
      
      if (cloudAccounts) {
        for (const acc in cloudAccounts) {
          accounts.push(acc);
          passwords.push(cloudAccounts[acc]);
        }
      }
      
      accounts.push(username);
      passwords.push(password);
      
      console.log('注册 - 发送账号列表:', accounts);
      console.log('注册 - 发送密码列表:', passwords);
      
      const config = {
        productID: app.globalData.productID,
        deviceID: app.globalData.deviceID,
        authorization: app.globalData.authorization
      };
      
      // 直接设置属性
      let success = await this.trySetProperty(config, accounts, passwords);

      wx.hideLoading();

      if (success) {
        // 更新本地缓存
        const newCloudAccounts = { ...cloudAccounts };
        newCloudAccounts[username] = password;
        this.setData({ cloudAccounts: newCloudAccounts });
        
        // 同时保存到本地存储，下次进入时优先使用
        wx.setStorageSync('localAccounts', newCloudAccounts);
        
        wx.showToast({ title: '注册成功', icon: 'success' });
        
        setTimeout(() => {
          this.setData({ isLogin: true, password: '', confirmPassword: '' });
        }, 1500);
      } else {
        wx.showToast({ title: '注册失败，请重试', icon: 'none' });
      }
    } catch (err) {
      wx.hideLoading();
      console.error('注册失败:', err);
      wx.showToast({ title: '网络错误', icon: 'none' });
    } finally {
      this.setData({ loading: false });
    }
  },

  // 设置属性到云平台
  // 设置属性到云平台
  async trySetProperty(config, accounts, passwords) {
    const url = `${app.globalData.serverUrl}/thingmodel/set-device-property`;
    
    try {
      const res = await new Promise((resolve, reject) => {
        wx.request({
          url,
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
          success: (res) => {
            console.log('注册响应:', res.data);
            resolve(res);
          },
          fail: reject
        });
      });
      
      return res.data?.code === 0;
    } catch (err) {
      console.log('注册失败:', err);
      return false;
    }
  }
});
