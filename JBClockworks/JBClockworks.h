#include <Arduino.h>
#include <Streaming.h>
#include "JBUtils.h"

#ifndef JBClockWorks_VERSION

#define JBClockWorks_VERSION "2021-05-28"

class JBCountDown {
private:
    uint32_t start_millis=0;
    uint32_t now_millis=0;
    uint32_t NbCountDownMillis;
public:
    uint32_t initNbSec, MillisLeft, SecLeft, MinLeft, HoursLeft;
    bool Started=false;

    JBCountDown(uint32_t nbSeconds){
        initNbSec=nbSeconds;
        NbCountDownMillis=nbSeconds*1000;
        Update();
    }

    void Reset(){
        Stop();
        start_millis=0;
        now_millis=0;
        Update();
    }

    bool Start(){
        Started=true;
        start_millis=millis();
        MillisLeft=;
    }
    void Stop(){Started=false;}

    // Update() returns true if there is time left (it returns Started bool value).
    bool Update(){
        
        MinLeft=SecLeft/60;
        HoursLeft=MinLeft/60;
        if(Started){
            now_millis=millis();
            SecLeft=(now_millis-start_millis)/1000;
            if(now_millis-start_millis>=)


        }
        return Started;
    }


};




#endif