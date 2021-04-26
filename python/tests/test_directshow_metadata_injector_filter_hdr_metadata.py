import unittest

from directshow_metadata_injector_filter.hdr_metadata import HDRMetaData


class TestHDRMetaData(unittest.TestCase):

    def test_getters(self):
        hdr = HDRMetaData("87:01:1A:90:02:00:34:21:AA:9B:96:19:FC:08:48:8A:08:39:13:3D:42:40:E8:03:32:00:F7:03:83:00:00")

        self.assertEqual(2, hdr.etof())

        self.assertAlmostEqual(0.3127, hdr.whitepoint_x())
        self.assertAlmostEqual(0.329, hdr.whitepoint_y())

        self.assertAlmostEqual(1000, hdr.mastering_luminance_max())
        self.assertAlmostEqual(0.005, hdr.mastering_luminance_min())

        self.assertEqual(1015, hdr.luminance_max_cll())
        self.assertEqual(131, hdr.luminance_max_fall())

        self.assertAlmostEqual(0.17, hdr.display_primary_green_x())
        self.assertAlmostEqual(0.797, hdr.display_primary_green_y())
        self.assertAlmostEqual(0.131, hdr.display_primary_blue_x())
        self.assertAlmostEqual(0.046, hdr.display_primary_blue_y())
        self.assertAlmostEqual(0.708, hdr.display_primary_red_x())
        self.assertAlmostEqual(0.292, hdr.display_primary_red_y())
