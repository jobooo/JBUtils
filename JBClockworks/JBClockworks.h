#include <Arduino.h>
#include <Streaming.h>
#include "JBUtils.h"

#ifndef JBClockWorks_VERSION

#define JBClockWorks_VERSION "2021-05-29"
#define Minutes *60*1000

enum ClockResolution   {
    H,  // Hours
    M,  // Minutes
    S,  // Seconds
    ms  // MilliSeconds
};

class JBCountDown {
private:
    uint32_t start_millis=0;
    uint32_t NbCountDownMillis;
public:
    uint32_t MillisLeft, SecLeft, MinLeft, HoursLeft;
    bool Started=false;
    bool EndReached=false;
    ClockResolution Resolution=M;

    uint32_t LastValue =0; 

    JBCountDown(uint32_t nbSeconds, ClockResolution pRes=M){
        NbCountDownMillis=nbSeconds*1000;
        Resolution=pRes;
        Reset();
    }

    void Reset(){
        Stop();
        start_millis=0;
        MillisLeft=NbCountDownMillis;
        SecLeft=MillisLeft/1000;
        MinLeft=SecLeft/60;
        HoursLeft=MinLeft/60;
        EndReached=false;
    }

    void Start(){
        Started=true;
        start_millis=millis();
        MillisLeft=NbCountDownMillis;
    }

    void Stop(){
        Started=false;
    }

    bool HasChanged(){
        Update();
        switch (Resolution){
            case ms:
                if(LastValue!=MillisLeft) {
                    LastValue=MillisLeft;
                    return true;
                }
            break;
            case S:
                if(LastValue!=SecLeft) {
                    LastValue=SecLeft;
                    return true;
                }
            break;
            case M:
                if(LastValue!=MinLeft) {
                    LastValue=MinLeft;
                    return true;
                }
            break;
            case H:
                if(LastValue!=HoursLeft) {
                    LastValue=HoursLeft;
                    return true;
                }
            break;
            default:
                return false;
            break;
        }
        return false;
    }

    // Update() returns true if there is time left (it returns Started bool value).
    bool Update(){
        if(Started){
            MillisLeft=NbCountDownMillis - (millis()-start_millis);
            if(MillisLeft <= 0){
                MillisLeft=SecLeft=MinLeft=HoursLeft=0;
                EndReached=true;
                return false;
            }else{
                SecLeft=MillisLeft/1000;
                MinLeft=SecLeft/60;
                HoursLeft=MinLeft/60;
            }
        }
        return true;
    }
};

#endif