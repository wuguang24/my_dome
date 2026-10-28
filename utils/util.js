// utils/util.js
function parseToolStatus(bitmap, tools) {
  return tools.map(tool => ({
    ...tool,
    isBorrowed: Boolean(bitmap & (1 << (tool.index - 1))),
    statusText: (bitmap & (1 << (tool.index - 1))) ? '使用中' : '空闲',
    statusColor: (bitmap & (1 << (tool.index - 1))) ? '#ff4d4f' : '#52c41a'
  }));
}

function formatTime(date) {
  const year = date.getFullYear();
  const month = date.getMonth() + 1;
  const day = date.getDate();
  const hour = date.getHours();
  const minute = date.getMinutes();
  const second = date.getSeconds();

  return `${[year, month, day].map(formatNumber).join('/')} ${[hour, minute, second].map(formatNumber).join(':')}`;
}

function formatNumber(n) {
  n = n.toString();
  return n[1] ? n : `0${n}`;
}

module.exports = {
  parseToolStatus,
  formatTime
};
