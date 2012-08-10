import unittest
import molequeue.utils

class TestUtils(unittest.TestCase):

  def setUp(self):
    unittest.TestCase.setUp(self)
    self.test_values = [('this_is_a_test', 'thisIsATest'),
                        ('this', 'this')]

  def test_underscore_to_camelcase(self):
    for (underscores, camelcase) in self.test_values:
      self.assertEqual(molequeue.utils.underscore_to_camelcase(underscores),
                       camelcase)

  def test_camelcase_to_underscore(self):
    for (underscores, camelcase) in self.test_values:
      self.assertEqual(molequeue.utils.camelcase_to_underscore(camelcase),
                       underscores)

if __name__ == '__main__':
    unittest.main()
