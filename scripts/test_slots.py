"""Safety checks for metadata writes on the two-slot device."""
import binascii
import struct
import unittest
from slot_tool import records, selection, validate_layout


def data(seq=1, state=2):
    crc = binascii.crc32(struct.pack('<I', seq), 0xffffffff) & 0xffffffff
    return struct.pack('<I20sII', seq, b'\xff' * 20, state, crc) + b'\xff' * (8192 - 32)


class MetadataTests(unittest.TestCase):
    def test_original_crc_and_fallback(self):
        before = data()
        self.assertEqual(before[28:32], bytes.fromhex('9a984347'))
        sector, content = selection(before, 'B')
        self.assertEqual(sector, 1)
        after = before[:4096] + content
        self.assertEqual(after[:4096], before[:4096])
        self.assertEqual(records(after)[1], dict(sector=1, seq=2, state=0, valid=True))

    def test_noop_and_unconfirmed_rejected(self):
        self.assertIsNone(selection(data(), 'A'))
        for state in (0, 1):
            with self.assertRaises(ValueError):
                selection(data(state=state), 'B')

    def test_corruption_and_erased_rejected(self):
        for content in (b'\xff' * 8192, data()[:28] + b'\x00' * 4 + data()[32:], b''):
            with self.assertRaises(ValueError):
                selection(content, 'B')

    def test_sequence_overflow(self):
        with self.assertRaises(ValueError):
            selection(data(seq=0xfffffffe), 'A')

    def test_layout_mismatch_rejected(self):
        with self.assertRaises(ValueError):
            validate_layout(b'\xff' * 4096)


if __name__ == '__main__':
    unittest.main()
