var telem_url = "replay";
var dochash = document.location.hash;
if (/^#\d\d+$/.exec(dochash)) {
    telem_url = "serial/" + dochash.substring(1);
}

function ll2str(v, plus, minus) {
    direc = plus;
    if (v < 0) {
        v = -v;
        direc = minus;
    }
    return v.toFixed(6) + "&deg;" + direc;
}

var tile_size_nmi = 0.75;
var px_size = 512;
function pt_z(latlng, zoom) {
    scale_x = 256/360.0 * Math.pow(2, zoom);
    scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);
    return pt_sxy(latlng, scale_x, scale_y);
}
function pt_sxy(latlng, scale_x, scale_y) {
    res = {};
    res.x = latlng.lng * scale_x;
    e = Math.sin(Math.PI*latlng.lat/180.0);
    e = Math.max(e,-(1-1.0E-15));
    e = Math.min(e,  1-1.0E-15 );
    res.y = 0.5*Math.log((1+e)/(1-e))*-scale_y;
    return res;
}


var height = 512*3;
var width = 512*3;

var visheight = height; // window.innerHeight;
var viswidth = width; // window.innerWidth;

var paper = new Raphael(document.getElementById('canvas_container'),
            viswidth, visheight);

var offx = 0; // -520;
var offy = 0; // -700;

centre = {"lat": -27.843933, "lng": 152.957153};
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.834219,E152.946167-16.png",    0+offx,    0+offy, 512,512);
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.834219,E152.957153-16.png",  512+offx,    0+offy, 512,512);
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.834219,E152.968140-16.png", 1024+offx,    0+offy, 512,512);
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.843933,E152.946167-16.png",    0+offx,  512+offy, 512,512);
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.843933,E152.957153-16.png",  512+offx,  512+offy, 512,512);
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.843933,E152.968140-16.png", 1024+offx,  512+offy, 512,512);
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.853647,E152.946167-16.png",    0+offx, 1024+offy, 512,512);
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.853647,E152.957153-16.png",  512+offx, 1024+offy, 512,512);
paper.image("http://azure.humbug.org.au/~aj/tmp/amazon-rockets/maps/map-S27.853647,E152.968140-16.png", 1024+offx, 1024+offy, 512,512);

statecols = ["white", "white", "gray", "red", "pink", "yellow", "lightblue", "blue", "darkgreen"]

var els = document.getElementsByName('info');
for (var i = 0; i < els.length; i++) {
    els.item(i).style.visibility = 'hidden';
}

function check(cond) {
    return cond ? " &#x2714;" : " &#x2718;";
}

function adc_volts(adc, range, good) {
    v = adc * range / 32768;
    return v.toFixed(1) + " V" + check(v >= good);
};

function insertAfter( referenceNode, newNode )
{
    referenceNode.parentNode.insertBefore( newNode, referenceNode.nextSibling );
}

function update_data(x) {
    if (x.statename == 'pad') {
        vis = 'pad';
    } else if (x.statename == 'boost' || x.statename == 'fast' || x.statename == 'coast') {
        vis = 'ascent';
    } else {
        vis = 'descent';
    }
    var base = document.getElementById('baseinfo');
    var els = document.getElementsByName('info');
    for (var i = 0; i < els.length; i++) {
        if (els.item(i).id == vis) {
            els.item(i).style.visibility = 'visible';
            insertAfter(base, els.item(i));
        } else {
            els.item(i).style.visibility = 'hidden';
        }
    }

    accel_to_mss = (x.cal_a2 - x.cal_a1)/2.0/9.80665;
    document.getElementById('callsign').innerHTML = x.callsign;
    document.getElementById('rssi').innerHTML = x.rssi;
    document.getElementById('serial').innerHTML = x.serial;
    document.getElementById('flight').innerHTML = x.flight;

    try {
    document.getElementById('nsats').innerHTML = x.gpsn + check(x.gpsn >= 4);
    document.getElementById('battery').innerHTML = x.battery.toFixed(1) + "V" + check(x.battery >= 3.7);
        // adc_volts(x.adc_v, 5.0, 3.7);
    document.getElementById('drogue').innerHTML = x.drogue.toFixed(1) + "V" + check(x.drogue >= 3.2);
        // adc_volts(x.adc_d, 15.0, 3.2);
    document.getElementById('main').innerHTML = x.main.toFixed(1) + "V" + check(x.main >= 3.2);
        // adc_volts(x.adc_m, 15.0, 3.2);
    } catch(e) { }

    try {
    document.getElementById('state').innerHTML = x.statename;
    document.getElementById('height').innerHTML = x.height.toFixed(0) + " m (" + (x.height * 3.28084).toFixed(0) + " ft)";
    document.getElementById('accel').innerHTML =
        x.acceleration.toFixed(2) + " m/s<sup>2</sup>";
    document.getElementById('speed').innerHTML =
        x.speed.toFixed(2) + " m/s";
    document.getElementById('baro_speed').innerHTML =
        x.baro_speed.toFixed(2) + " m/s";
    document.getElementById('lat').innerHTML = ll2str(x.gps.lat, "N", "S");
    document.getElementById('long').innerHTML = ll2str(x.gps.lon, "E", "W");

    when = new Date(x.gps.Y, x.gps.M-1, x.gps.D, x.gps.h, x.gps.m, x.gps.s, 0);
    document.getElementById('time').innerHTML = when.toLocaleTimeString();
    } catch(e) {
        document.getElementById('state').innerHTML = 'NOSTATE';
    }

    if (x.gps.lat == 0 && x.gps.lon == 0)
        return;

    ll = {"lat": x.gps.lat, "lng": x.gps.lon};
    ll = pt_z(ll, 16);
    llc = pt_z(centre, 16);
    cx = ll.x - llc.x + 512 + 256;
    cy = ll.y - llc.y + 512 + 256;
    var circle = paper.circle(cx+offx, cy+offy, 4);
    circle.attr({fill: "black", stroke: statecols[x.state], "stroke-width": 4});

    myscroll(cx, cy);
}

var tx = 0;
var ty = 0;
function myscroll(cx, cy) {
    if (cx < tx || cy < ty || tx + window.innerWidth < cx || ty + window.innerHeight < cy) {
        tx = cx-window.innerWidth/2;
        ty = cy-window.innerHeight/2;
        if (tx < 0) tx = 0;
        if (ty < 0) ty = 0;
        window.scrollTo(tx, ty);
    }
}

var JSON;
if (!JSON) {
    JSON = {};
    JSON.parse = function (s) {
        return eval( "("+s+")" );
    }
}

var updates = 0;
function updatetelem() {
    try {
        xmlhttp = new XMLHttpRequest();
        xmlhttp.open("GET",telem_url,false);
        xmlhttp.send();
        var resp = xmlhttp.responseText;
        update_data(JSON.parse(resp));
        t=setTimeout("updatetelem()",500);
    } catch (e) {
        t = setTimeout("updatetelem()", 100);
    }
}

window.onload = function () {
    setTimeout("updatetelem()",1000);
}
