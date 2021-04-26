import unittest
from directshow_metadata_injector_filter import vic


class TestVic(unittest.TestCase):

    def testVic32(self):
        data = vic.vic_by_id(32)
        self.assertEqual('1080p24', data['name'])
        self.assertEqual('16:9', data['display_aspect_ratio'])
