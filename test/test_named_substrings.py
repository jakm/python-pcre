import unittest
import pcre

class TestNamedSubstrings(unittest.TestCase):
    def setUp(self):
        pattern = r'(?<date>(?<year>(\d\d)?\d\d) - (?<month>\d\d) - (?<day>\d\d))'
        self.regex = pcre.compile(pattern)
    
    def test_groups_count(self):
        self.assertEquals(5, self.regex.groups)
    
    def test_named_groups_indexes(self):
        self.assertTrue('date' in self.regex.groupindex)
        self.assertEquals(1, self.regex.groupindex['date'])
        
        self.assertTrue('year' in self.regex.groupindex)
        self.assertEquals(2, self.regex.groupindex['year'])
        
        self.assertTrue('day' in self.regex.groupindex)
        self.assertEquals(5, self.regex.groupindex['day'])
        
        self.assertTrue('month' in self.regex.groupindex)
        self.assertEquals(4, self.regex.groupindex['month'])

if __name__ == '__main__':
    unittest.main()