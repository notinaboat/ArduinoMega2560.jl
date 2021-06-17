"""
Julia interface for Aurduino Mega 2560.
"""
module ArduinoMega2560

using Once
using PiAVR
using PiAVRDude



# Firmware Programming.

function flash(port)

        avr = AVRDevice(device = "atmega2560",
                      usb_port = port,
                        c_file = joinpath(@__DIR__, "main.c"),
                         fuses = [])

        PiAVR.flash(avr)
end



# GPIO Interface.

struct MegaGPIO

    port::String
    rx::Channel{String}
    tx::IOStream

    function MegaGPIO(port)

        # Open rx channel.
        rx = readline_channel(port)

        # Open tx stream.
        tx = open(port, read=false, write=true)

        # Configrue serial port.
        run(`stty -F $port 38400 raw -echo -onlcr`)

        # Wait for DTR auto-reset.
        sleep(1)

        m = new(port, rx, tx)

        # Send three dummy commands.
        for i in 0:3
            send(m, "IA$i")
            sleep(0.1)
        end

        # Wait for response to dummy commands.
        while !startswith(take!(m.rx), "IA3")
        end

        m
    end
end


function readline_channel(port)
    channel = Channel{String}(0)
    io = open(`cat $port`, read=true, write=false)
    @async try
        while true
            @assert(isopen(io.out))
            line = readline(io)
            put!(channel, line)
            yield()
        end
    catch err
        exception=(err, catch_backtrace())
        @error "Error reading from $port" exception
    end

    return channel
end


status(m::MegaGPIO) = PiAVRDude.status(m.avr.isp)


function send(m::MegaGPIO, data)
    write(m.tx, data, "\r\n")
    flush(m.tx)
end


function command(m::MegaGPIO, command)
    send(m, command)
    result = take!(m.rx)
    @assert startswith(result, command)
    value = result[length(command)+1:end]
    return parse(Int, value; base = 16)
end


output_low(m, pin) = (command(m, "L$pin"); nothing)
output_high(m, pin) = (command(m, "H$pin"); nothing)
enable_input_with_pullup(m, pin) = (command(m, "U$pin"); nothing)
enable_input_without_pullup(m, pin) = (command(m, "D$pin"); nothing)
read_input(m, pin) = !iszero(command(m, "I$pin"))

function enable_input(m::MegaGPIO, pin; pullup=false)
    pullup ? enable_input_with_pullup(m, pin) :
             enable_input_without_pullup(m, pin)
end

Base.setindex!(m::MegaGPIO, v::Bool, pin) = v ? output_high(m, pin) :
                                                output_low(m, pin)
Base.setindex!(m::MegaGPIO, v, pin) = setindex!(p, !iszero(v), pin)
Base.getindex(m::MegaGPIO, pin) = !iszero(read_input(m, pin))



# ADC Interface.

struct MegaADC
    gpio::MegaGPIO
end

Base.getindex(m::MegaADC, pin) = command(m.gpio, "A$pin")



end # module
