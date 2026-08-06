def test_ping(dc):
    r = dc.ping()
    assert r["dc"] == 1
    assert r["fw"].startswith("Rev ")


def test_dump_shapes(dc):
    assert "params" in dc.dump("params")
    chan = dc.dump("chan")["chan"]
    assert len(chan) == 4
    flags = dc.dump("flags")
    for k in ("cf", "vu", "kdu", "home", "wfm", "step", "sql", "vol"):
        assert k in flags
