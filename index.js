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
    close(){
        nativeKinect2.close();
    }

    openTracker(){
        nativeKinect2.openTracker(this.trackerCallback.bind(this));
    }

    trackerCallback(data){        
        if(data.hasOwnProperty("hasBody")){
            this.emit("has_body", data.hasBody);
        }
        if(data.hasOwnProperty("users")){
            this.emit("body_info", data.users);
        }        
    }
}



module.exports = Kinect2;