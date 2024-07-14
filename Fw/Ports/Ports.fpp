module Components {

    enum Channel : U8 {
            Ch0     =   0,   
            Ch1     =   1,
            Ch2     =   2,
            Ch3     =   3,
    }

    enum ADCChannel : U8 {
        Ch0     =   0,   
        Ch1     =   1,
    }

    enum Port   : U8 {
        P0     =   0,   
        P1     =   1,
        P2     =   2,
        P3     =   3,
    }

    enum InternalChannel : U8 {
        MPU     =   1,
        Int5V   =   2,
        Int3V3  =   3,
    }

    port PowerRequest (
        channel : Channel
        $port : Port
        power : bool
    )

    port AutomatonRequest(
        provider : string
        payload : string
        service : string
        image : string
        action : string
        arg : string
    )

    port PktAck (
        ref packetID: U8
        context : U32
    )

}