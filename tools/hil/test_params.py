def test_sql_roundtrip(dc):
    orig = dc.get("sql")
    try:
        new = "3" if orig != "3" else "4"
        dc.set("sql", new)
        assert dc.get("sql") == new
    finally:
        dc.set("sql", orig)


def test_volume_roundtrip(dc):
    orig = dc.get("volume")
    try:
        new = "2" if orig != "2" else "3"
        dc.set("volume", new)
        assert dc.get("volume") == new
    finally:
        dc.set("volume", orig)
