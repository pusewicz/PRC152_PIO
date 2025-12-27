# PRC-152 Radio UI - Main Ruby Application
# This file is compiled to bytecode and runs on the mruby/c VM

# Global instances (created by C bindings)
$lcd = LCD.new
$radio = Radio.new
$event = Event.new
$timer = Timer.new
$system = System.new

# ============================================================================
# UI Widgets
# ============================================================================

class StatusBar
  def render
    # R/T indicator and battery
    $lcd.text(0, 0, "R BAT", :small)
    $lcd.battery($radio.battery)

    # Labels
    $lcd.text(48, 0, "VULOS MOI", :small)
    $lcd.text(88, 0, "------- PT", :small)

    # Volume bars
    $lcd.volume($radio.volume)
  end
end

class Menu
  def initialize(items)
    @items = items
    @position = 0
    @page_size = 3
  end

  def render
    $lcd.clear(:edit_zone)

    page = @position / @page_size
    start_idx = page * @page_size

    @page_size.times do |i|
      idx = start_idx + i
      break if idx >= @items.length

      selected = (idx == @position)
      y = i + 1
      $lcd.text(0, y, @items[idx], :medium, selected)
    end

    $lcd.scrollbar(@items.length, @position, @page_size)
  end

  def up
    @position = @position - 1
    @position = @items.length - 1 if @position < 0
    render
  end

  def down
    @position = @position + 1
    @position = 0 if @position >= @items.length
    render
  end

  def selected_index
    @position
  end

  def selected_item
    @items[@position]
  end
end

# ============================================================================
# Screens
# ============================================================================

class MainScreen
  MODE_MAIN = 0
  MODE_BIG  = 1
  MODE_DUAL = 2

  def initialize
    @mode = MODE_MAIN
    @status_bar = StatusBar.new
  end

  def render
    @status_bar.render

    case @mode
    when MODE_MAIN
      render_main_mode
    when MODE_BIG
      render_big_mode
    when MODE_DUAL
      render_dual_mode
    end

    render_signal
  end

  def render_main_mode
    chan = $radio.channel
    chan_num = chan[:number]

    # Band indicator
    if chan_num == 100
      band = "UHF "
    elsif chan_num == 0
      band = "VHF "
    else
      band = "CHAN"
    end
    $lcd.text(66, 1, band, :medium)

    # Frequency display
    if $radio.ptt?
      $lcd.frequency(0, 1, chan[:rx_freq])
    else
      $lcd.frequency(0, 1, chan[:tx_freq])
    end

    # Channel number
    $lcd.channel(83, 2, chan_num)

    # Duplex indicator
    if chan[:rx_freq] != chan[:tx_freq]
      $lcd.icon(92, 1, 0)  # duplex icon
    end

    # Bottom info line
    $lcd.text(0, 3, "TYPE   TRF    MOD    CHAN  KEY", :small)
    $lcd.text(0, 2, "LOS", :medium)
  end

  def render_big_mode
    chan = $radio.channel
    label = sprintf("%02d-%s", chan[:number], chan[:name])
    $lcd.text(0, 1, label, :large)
    $lcd.text(0, 3, "THE NICKNAME OF YOUR CHANNEL.", :small)
  end

  def render_dual_mode
    $lcd.text(0, 1, "A: CH-01 ALPHA  ", :medium)
    $lcd.text(0, 2, "B: CH-02 BRAVO  ", :medium)
    $lcd.text(0, 3, "DUAL STANDBY MODE.", :small)
  end

  def render_signal
    if $radio.ptt?
      $lcd.char(0, 0, 82)  # 'R' for receive
      $lcd.signal($radio.rssi)
    else
      $lcd.char(0, 0, 84)  # 'T' for transmit
      $lcd.signal(100)
    end
  end

  def cycle_mode
    @mode = (@mode + 1) % 3
  end

  def on_event(evt)
    case evt[:type]
    when 1  # KEY_CLICK
      cycle_mode
      render
    when 2  # KEY_LONG
      $system.log("Long press - shutdown requested")
    end
  end
end

# ============================================================================
# Application Entry Point
# ============================================================================

class App
  def initialize
    @screen = MainScreen.new
    $system.log("Ruby UI initialized")
    $system.log("Free memory: " + $system.free_memory.to_s)
  end

  def run
    @screen.render

    # Main event loop
    loop do
      evt = $event.poll
      if evt
        @screen.on_event(evt)
      end

      # Cooperative yield - let C code run
      $timer.delay(50)

      # Periodic refresh
      @screen.render
    end
  end
end

# Start the application
app = App.new
app.run
