"use strict";
const nativeKinect2 = require('bindings')('hoowu_kinect2');
const EventEmitter = require("events").EventEmitter;

class Kinect2 extends EventEmitter{
    constructor(){
        super();
    }

    open(){
        return nativeKinect2.open();
    }

    openTracker(){
        nativeKinect2.openTracker(this.trackerCallback.bind(this));
    }

    trackerCallback(data){
        this.emit("track", data);
    }
}



module.exports = Kinect2;