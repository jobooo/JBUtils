#include <Arduino.h>
#include <Streaming.h>
#include "JBUtils.h"

#ifndef JBClockWorks_VERSION

#define JBClockWorks_VERSION "2021-05-29"

class JBCountDown {
private:
    uint32_t start_millis=0;
    uint32_t NbCountDownMillis;
public:
    uint32_t MillisLeft, SecLeft, MinLeft, HoursLeft;
    bool Started=false;

    JBCountDown(uint32_t nbSeconds){
        NbCountDownMillis=nbSeconds*1000;
        Reset();
    }

    void Reset(){
        Stop();
        start_millis=0;
        MillisLeft=NbCountDownMillis;
        SecLeft=MillisLeft/1000;
        MinLeft=SecLeft/60;
        HoursLeft=MinLeft/60;
    }

    bool Start(){
        Started=true;
        start_millis=millis();
        MillisLeft=NbCountDownMillis;
    }

    void Stop(){
        Started=false;
    }

    // Update() returns true if there is time left (it returns Started bool value).
    bool Update(){
        if(Started){
            MillisLeft=NbCountDownMillis - (millis()-start_millis);
            if(MillisLeft <= 0){
                Started=false;
                MillisLeft=SecLeft=MinLeft=HoursLeft=0;
                Started=false;
            }else{
                SecLeft=MillisLeft/1000;
                MinLeft=SecLeft/60;
                HoursLeft=MinLeft/60;
            }
        }
        return Started;
    }
};

#endif