target remote :3333
monitor arm semihosting enable
file build/moyos.elf
monitor reset halt
layout split

# For easy resetting
define halt
    monitor reset halt
end

# For easy reloading
define reload
    halt
    load build/moyos.elf
    file build/moyos.elf
end

