from dc_client import render_screen


def test_screen_not_blank(dc):
    data = dc.screen()
    assert len(data) == 1024
    assert any(data), "shadow buffer is all zeros - LCD mirroring broken?"


def test_screen_render_smoke(dc):
    art = render_screen(dc.screen())
    lines = art.splitlines()
    assert len(lines) == 64
    assert all(len(l) == 128 for l in lines)
    print(art)  # visual aid: pytest -s shows the display
