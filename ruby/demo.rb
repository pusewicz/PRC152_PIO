# Simple demo to verify mruby/c integration works
# This is a minimal test before running the full UI

$lcd = LCD.new
$radio = Radio.new
$system = System.new
$timer = Timer.new

$system.log("=== Ruby Demo Starting ===")
$system.log("Free heap: " + $system.free_memory.to_s + " bytes")

# Clear and show hello message
$lcd.clear
$lcd.text(20, 1, "Ruby Radio!", :medium)
$lcd.text(10, 2, "mruby/c works!", :medium)

# Show battery and volume
$lcd.text(0, 0, "BAT", :small)
$lcd.battery($radio.battery)
$lcd.volume($radio.volume)

# Wait then show channel info
$timer.delay(2000)

chan = $radio.channel
$lcd.clear(:edit_zone)
$lcd.text(0, 1, "CH:", :medium)
$lcd.channel(20, 1, chan[:number])
$lcd.frequency(0, 2, chan[:rx_freq])

$system.log("Channel: " + chan[:number].to_s)
$system.log("RX Freq: " + chan[:rx_freq].to_s)
$system.log("=== Demo Complete ===")
