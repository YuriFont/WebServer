from pathlib import Path
import os

def ensure_test_file(
    path: Path,
    size_mb: int,
    randomize: bool = False,
    pattern: bytes = b"A"
):
    path.parent.mkdir(parents=True, exist_ok=True)

    if path.exists() and path.stat().st_size == size_mb * 1024 * 1024:
        return

    total_size = size_mb * 1024 * 1024

    with open(path, "wb") as f:
        remaining = total_size
        while remaining > 0:
            if randomize:
                chunk = os.urandom(min(4096, remaining))
            else:
                chunk = pattern * min(4096, remaining)

            f.write(chunk)
            remaining -= len(chunk)
