let hits = 0;
const hitElement = document.querySelector('.hits');

document.body.onkeyup = function (e) {
  if (e.keyCode == 32) {
    addHit();
  }
};

window.webview.onMessage = (message) => {
  console.log(`Message from C++ side: ${message}`);
};

const addHit = () => {
  hits++;
  if (window.counter_value) {
    window.counter_value(hits);
  }
  renderHits();
};

const renderHits = () => {
  hitElement.innerHTML = hits;
};

const resetCounter = () => {
  hits = 0;
  renderHits();
  if (window.counter_value) {
    window.counter_reset(hits);
  }
};
