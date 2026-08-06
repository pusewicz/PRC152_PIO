import time


def test_channel_step_updates_state(dc):
    """UP then DOWN on the VFO page: channel changes and comes back."""
    before = dc.dump("chan")["chan"][0]["chan"]
    dc.key("UP")
    time.sleep(0.4)
    after = dc.dump("chan")["chan"][0]["chan"]
    dc.key("DOWN")
    time.sleep(0.4)
    restored = dc.dump("chan")["chan"][0]["chan"]
    assert after != before
    assert restored == before


def test_menu_open_close_via_injection(dc):
    """ENT opens a blocking menu; CLR exits — console must stay alive inside."""
    dc.key("ENT")
    time.sleep(0.4)
    assert dc.ping()["ok"] == 1   # poll placement keeps console alive in menus
    dc.key("CLR")
    time.sleep(0.4)
    assert dc.ping()["ok"] == 1
