import unittest


@unittest.skip("use algo/tests/test_level_integration.py with ZAPPY_RUN_LIVE=1")
class RefLevel4IntegrationTests(unittest.TestCase):
    def test_deprecated_shell_harness(self) -> None:
        self.fail("shell harness removed; use algo/tests/test_level_integration.py")


if __name__ == "__main__":
    unittest.main()
