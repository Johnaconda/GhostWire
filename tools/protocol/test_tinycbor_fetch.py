#!/usr/bin/env python3
"""No external network: ensure explicit upstream retrieval fails closed."""
import hashlib
import io
from pathlib import Path
import tempfile
import zipfile

import fetch_tinycbor as fetch


class FakeReply:
    def __init__(self, data, url=fetch.URL):
        self.stream = io.BytesIO(data)
        self.url = url
    def geturl(self):
        return self.url
    def read(self, size):
        return self.stream.read(size)
    def __enter__(self):
        return self
    def __exit__(self, *_):
        pass


def zip_bytes(payload):
    out = io.BytesIO()
    with zipfile.ZipFile(out, 'w', compression=zipfile.ZIP_DEFLATED) as z:
        z.writestr('tinycbor-pinned/src/file.h', payload)
    return out.getvalue()


def expect_failure(label, data, pins, url=fetch.URL):
    with tempfile.TemporaryDirectory() as td:
        dest = Path(td) / 'result.zip'
        try:
            fetch.retrieve_verified(dest, pins, lambda req, timeout: FakeReply(data, url))
        except (OSError, ValueError, zipfile.BadZipFile):
            assert not dest.exists(), label
        else:
            raise AssertionError(f'{label}: invalid source was accepted')


def main():
    content = b'pretend upstream source'
    pins = {'files': {'src/file.h': [len(content), hashlib.sha256(content).hexdigest()]}}
    good = zip_bytes(content)
    with tempfile.TemporaryDirectory() as td:
        output = Path(td) / 'upstream.zip'
        assert fetch.retrieve_verified(output, pins,
            lambda req, timeout: FakeReply(good)) == len(good)
        assert output.read_bytes() == good
        # Never overwrite previously accepted source.
        try:
            fetch.retrieve_verified(output, pins, lambda req, timeout: FakeReply(good))
        except ValueError:
            pass
        else:
            raise AssertionError('output was overwritten')
        assert output.read_bytes() == good
    expect_failure('tampered', zip_bytes(b'tampered'), pins)
    expect_failure('truncated', good[:-7], pins)
    expect_failure('redirect', good, pins, 'https://example.net/foreign')
    expect_failure('oversized', b'x' * (fetch.MAX_COMPRESSED + 1), pins)
    expect_failure('not-zip', b'garbage', pins)
    print('Explicit pinned upstream source downloader: PASS (6 offline scenarios)')


if __name__ == '__main__':
    main()
