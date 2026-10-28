// pages/records/records.js
Page({
  data: {
    records: [],
    filterType: 'all', // all, borrow, return
    filterIndex: 0,
    filterOptions: ['全部', '借用', '归还']
  },

  onLoad() {
    this.loadRecords();
  },

  onShow() {
    this.loadRecords();
  },

  loadRecords() {
    const allRecords = wx.getStorageSync('records') || [];
    const filterType = this.data.filterType;
    
    let records = allRecords;
    if (filterType !== 'all') {
      records = allRecords.filter(r => r.type === filterType);
    }
    
    this.setData({ records });
  },

  onFilterChange(e) {
    const index = parseInt(e.detail.value);
    const typeMap = ['all', 'borrow', 'return'];
    this.setData({ 
      filterIndex: index,
      filterType: typeMap[index] 
    });
    this.loadRecords();
  },

  // 清空记录
  clearRecords() {
    wx.showModal({
      title: '确认清空',
      content: '确定要清空所有记录吗？',
      success: (res) => {
        if (res.confirm) {
          wx.setStorageSync('records', []);
          this.setData({ records: [] });
          wx.showToast({ title: '已清空', icon: 'success' });
        }
      }
    });
  }
})
