"""
Julia interface for Aurduino Mega 2560.
"""
module ArduinoMega2560

using Once
using PiAVR
using PiAVRDude
using Sockets



# Firmware Programming.

function flash(port)

        avr = AVRDevice(device = "atmega2560",
                      usb_port = port,
                        c_file = joinpath(@__DIR__, "main.c"),
                         fuses = [])

        PiAVR.flash(avr)
end


function socat(from, to; debug=false)
    stderr_pipe=Pipe()
    cmd = debug ? `socat -v` : `socat`
    p = run(pipeline(`$cmd $from $to`;
                     stdin=devnull,
                     stdout=devnull,
                     stderr=stderr_pipe); wait = false)
    close(stderr_pipe.in)
    @async try
        while isopen(stderr_pipe.out)
            @warn "socat: $(readline(stderr_pipe))"
        end
        wait(p)
    catch err
        exception=(err, catch_backtrace())
        @error "Error reading from `socat`" exception
    end
end


# GPIO Interface.

struct MegaGPIO

    port::String
    rx::Channel{String}
    tx::IO
    monitor::Channel{String}
    usarts::Vector{Channel{String}}

    function MegaGPIO(port)

        socket = "$(tempname()).mega_gpio_socket"
        socat("$port,b38400,rawer", "UNIX-LISTEN:$socket"; debug=false)
        while !ispath(socket)
            @warn "Waiting for $socket"
            sleep(1)
        end
        io = Sockets.connect(socket)

        # Wait for DTR auto-reset.
        sleep(1)

        # Reset and drain input.
        send(io, "Z")
        x = recv(io)
        while x != ">Z" || bytesavailable(io) > 0
            x = recv(io)
        end

        # Reset again.
        send(io, "Z")
        r = recv(io)
        @assert r == ">Z"

        # Open rx channel.
        rx, monitor, usarts = readline_channels(io)

        m = new(port, rx, io, monitor, usarts)

        @info "Opened MegaGPIO on $port"

        m
    end
end

Base.close(m::MegaGPIO) = close(m.tx)
Base.isopen(m::MegaGPIO) = isopen(m.tx)

isfull(c::Channel) = length(c.data) ≥ c.sz_max

function readline_channels(io)
    rx_channel = Channel{String}(0)
    monitor_channel = Channel{String}(1000)
    usarts = [Channel{String}(1000) for i in 1:3]
    @async try
        while isopen(io)
            line = recv(io)
            if startswith(line, "1")
                @assert !isfull(usarts[1])
                put!(usarts[1], line[2:end])
            elseif startswith(line, "2")
                @assert !isfull(usarts[2])
                put!(usarts[2], line[2:end])
            elseif startswith(line, "3")
                @assert !isfull(usarts[3])
                put!(usarts[3], line[2:end])
            elseif startswith(line, "!")
                if length(line) != 4
                    @error "Bad monitor line $line"
                else
                    put!(monitor_channel, line[2:end])
                end
            elseif startswith(line, ">") 
                put!(rx_channel, line)
            else
                @error "Bad monitor line: \"$line\""
            end
            yield()
        end
        @warn "Mega.GPIO.readline_channels() exiting..."
    catch err
        exception=(err, catch_backtrace())
        @error "Error reading from $io" exception
    end

    return rx_channel, monitor_channel, usarts
end


status(m::MegaGPIO) = PiAVRDude.status(m.avr.isp)


function send(io, data)
    write(io, "$data\r\n")
    flush(io)
    #@info "\"$data\\r\\n\" => MegaGPIO"
    nothing
end

function recv(io)
    r = readline(io)
    if isempty(r) && isopen(io)
        r = readline(io)
    end
    #@info "MegaGPIO ==> \"$r\""
    r
end

function reset(m)
    send(m.tx, "Z")
    result = take!(m.rx)
    @assert result == ">Z"
    while !isempty(m.monitor)
        take!(m.monitor)
    end
    nothing
end

function command(m::MegaGPIO, command)
    send(m.tx, command)
    result = take!(m.rx)
    @assert startswith(result, ">$command")
    value = result[length(command)+2:end]
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
    usart::UInt8
end

rx_channel(m::MegaUSART) = m.gpio.usarts[m.usart]

Base.isempty(m::MegaUSART) = isempty(rx_channel(m))
Base.take!(m::MegaUSART) = take!(rx_channel(m))
Base.empty!(m::MegaUSART) = while !isempty(m) take!(m) end
Base.write(m::MegaUSART, x) = (command(m.gpio, "$(m.usart)$x") ; nothing)


end # module
