"""
Julia interface for Aurduino Mega 2560.
"""
module ArduinoMega2560

using Once
using PiAVR
using PiAVRDude
using UnixIO



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
    io::IO
    response::Channel{String}
    monitor::Channel{String}
    usarts::Vector{Channel{String}}

    function MegaGPIO(port)

   if false
        socket = "$(tempname()).mega_gpio_socket"
        @async UnixIO.system(`socat $port,b38400,rawer UNIX-LISTEN:$socket \< /dev/null`)

        while !ispath(socket)
            sleep(1)
            if !ispath(socket)
                @warn "Waiting for $socket"
            end
        end

            sleep(1)
        io = UnixIO.open(socket, UnixIO.O_RDWR; yield=false)

        sleep(1)
    else
        io = nothing
        @sync begin
            @async io = UnixIO.open(port,
                                    UnixIO.O_RDWR |
                                    UnixIO.O_NOCTTY |
                                    0x101000; # FIXME needed ? harmfull?
                                    yield=true)
            sleep(2)
            if io == nothing
                @error "Stuck trying to open $port"
            end
        end

        UnixIO.tcsetattr(io; lflag=UnixIO.ICANON, speed=38400)

        # Wait for DTR auto-reset.
        sleep(1)
        UnixIO.tcflush(io, UnicIO.TCIOFLUSH)

        response = Channel{String}(1000)
        monitor = Channel{String}(1000)
        usarts = [Channel{String}(1000) for i in 1:3]

        m = new(port, io, response, monitor, usarts)
        @info "Opened MegaGPIO on $port"
        reset(m)
        m
    end
end

Base.close(m::MegaGPIO) = close(m.io)
Base.isopen(m::MegaGPIO) = isopen(m.io)

isfull(c::Channel) = length(c.data) ≥ c.sz_max

status(m::MegaGPIO) = PiAVRDude.status(m.avr.isp)


function send_command(m, data)
    @info "\"$data\\r\\n\" => MegaGPIO"
    write(m.io, "$data\r\n")
    UnixIO.tcdrain(m.io) # FIXME needed?
    nothing
end

function recv_response(m)

    line = readline(m.io)
    while isempty(line) && isopen(m.io)
        line = readline(m.io)
    end
    if !isopen(m.io)
        throw(EOFError())
    end

    @assert !isempty(line)

    c = line[1]
    channel = if c == '>' m.response
          elseif c == '!' m.monitor
          elseif c == '1' m.usarts[1]
          elseif c == '2' m.usarts[2]
          elseif c == '3' m.usarts[3]
            else
                @error "MegaGPIO ==> \"$line\""
                return
            end

    @info "MegaGPIO ==> \"$line\""
    @assert !isfull(channel)
    put!(channel, line[2:end])

    nothing
end


empty_channel!(c::Channel) = while !isempty(c) take!(c) end


function reset(m)
    empty_channel!(m.response)
    send_command(m, "Z")
    while isempty(m.response)
        recv_response(m)
    end
    result = take!(m.response)
    @assert result == "Z"
    empty_channel!(m.monitor)
    for u in m.usarts
        empty_channel!(u)
    end
end

function command(m::MegaGPIO, command)
    send_command(m, command)
    while isempty(m.response)
        recv_response(m)
    end
    result = take!(m.response)
    @assert startswith(result, "$command")
    value = result[length(command)+1:end]
    return isempty(value) ? nothing : parse(Int, value; base = 16)
end


output_low(m, pin) = (command(m, "L$pin"); nothing)
output_high(m, pin) = (command(m, "H$pin"); nothing)
enable_input_with_pullup(m, pin) = (command(m, "U$pin"); nothing)
enable_input_without_pullup(m, pin) = (command(m, "D$pin"); nothing)
read_input(m, pin) = !iszero(command(m, "I$pin"))

enable_monitor(m, pin) = (command(m, "M$pin"); nothing)
disable_monitor(m, pin) = (command(m, "N$pin"); nothing)

function enable_input(m::MegaGPIO, pin; pullup=false)
    pullup ? enable_input_with_pullup(m, pin) :
             enable_input_without_pullup(m, pin)
end

Base.setindex!(m::MegaGPIO, v::Bool, pin) = v ? output_high(m, pin) :
                                                output_low(m, pin)
Base.getindex(m::MegaGPIO, pin) = read_input(m, pin)



# ADC Interface.

struct MegaADC
    gpio::MegaGPIO
end

Base.getindex(m::MegaADC, pin) = command(m.gpio, "A$pin")


# USART Interface.

struct MegaUSART
    gpio::MegaGPIO
    id::UInt8
end

rx_channel(m::MegaUSART) = m.gpio.usarts[m.id]

Base.isempty(m::MegaUSART) = isempty(rx_channel(m))
Base.empty!(m::MegaUSART) = empty_channel!(rx_channel(m))
Base.write(m::MegaUSART, x) = (command(m.gpio, "$(m.id)$x") ; nothing)

function Base.take!(m::MegaUSART) 
    c = rx_channel(m)
    while isempty(c)
        recv_response(m.gpio)
    end
    take!(c)
end


end # module
