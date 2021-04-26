import unittest

from directshow_metadata_injector_filter.avi_metadata import AVIMetaData


class TestAVIMetaData(unittest.TestCase):

    def test_getters(self):
        avi = AVIMetaData("82:02:0D:D5:32:E8:60:20:00:00:00:00:00:00:00:00:00")

        self.assertEqual(0, avi.csc())
        self.assertEqual(32, avi.vic())
