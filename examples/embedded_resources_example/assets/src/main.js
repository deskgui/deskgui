var hits = 0;
var hitElement = document.querySelector('.hits');
document.body.onkeyup = function (e) {
    if (e.keyCode == 32) {
        addHit();
    }
}

var addHit = function () {
    hits++;
    if (window.counter_value) {
        window.counter_value(hits);
    }
    renderHits();
}

var renderHits = function () {
    hitElement.innerHTML = hits;
}

var resetCounter = function () {
    hits = 0;
    renderHits();
    if (window.counter_value) {
        window.counter_reset(hits);
    }
}
