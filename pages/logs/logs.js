// pages/logs/logs.js
const util = require('../../utils/util.js');

Page({
  data: {
    logs: []
  },

  onShow() {
    this.fetchLogs();
  },

  async fetchLogs() {
    // 此处仅为演示，实际应从后端或OneNet事件API获取
    // 模拟一些日志数据
    this.setData({
      logs: [
        { timestamp: Date.now(), event_id: "BorrowEvent", params: { ToolIndex: 1 } },
        { timestamp: Date.now() - 3600000, event_id: "ReturnEvent", params: { ToolIndex: 3 } },
        { timestamp: Date.now() - 7200000, event_id: "TimeoutAlarm", params: { ToolIndex: 2 } }
      ]
    });
  }
});