import unittest
import pcre

class TestMatch(unittest.TestCase):
    def setUp(self):
        pattern = r'(?<date>(?<year>(\d\d)?\d\d) - (?<month>\d\d) - (?<day>\d\d))'
        self.regex = pcre.compile(pattern)
    
    def test_match(self):
        match = self.regex.match('99 - 01 - 01')
        self.assertTrue(match)
        self.assertTrue(isinstance(match, pcre._pcre.MatchObject))
    
    def test_nomatch(self):
        match = self.regex.match('some text')
        self.assertFalse(match)
    
    def test_match_re(self):
        match = self.regex.match('99 - 01 - 01')
        self.assertEquals(self.regex, match.re)
    
    def test_match_subject(self):
        subject = '99 - 01 - 01'
        match = self.regex.match(subject)
        self.assertEquals(subject, match.string)
    
    def test_match_group_without_args(self):
        subject = '99 - 01 - 01'
        match = self.regex.match(subject)
        self.assertEquals(subject, match.group())

if __name__ == '__main__':
    unittest.main()