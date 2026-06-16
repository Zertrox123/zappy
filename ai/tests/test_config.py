import unittest

from config import (
    DEFAULT_HOST,
    EXIT_USAGE,
    USAGE,
    AiConfig,
    ConfigParseError,
    parse_args,
)


def argv(*args: str) -> list[str]:
    return ["zappy_ai", *args]


class ParseSuccessTests(unittest.TestCase):
    def test_parse_exam_configuration(self) -> None:
        config = parse_args(argv("-p", "8080", "-n", "team"))
        self.assertEqual(
            config,
            AiConfig(port=8080, team_name="team", hostname=DEFAULT_HOST),
        )

    def test_parse_with_explicit_hostname(self) -> None:
        config = parse_args(argv("-p", "4242", "-n", "team", "-h", "127.0.0.1"))
        self.assertEqual(
            config,
            AiConfig(port=4242, team_name="team", hostname="127.0.0.1"),
        )

    def test_parse_flags_in_different_order(self) -> None:
        config = parse_args(argv("-h", "localhost", "-n", "team", "-p", "9000"))
        self.assertEqual(config.port, 9000)
        self.assertEqual(config.team_name, "team")
        self.assertEqual(config.hostname, "localhost")

    def test_parse_max_valid_port(self) -> None:
        config = parse_args(argv("-p", "65535", "-n", "team"))
        self.assertEqual(config.port, 65535)


class ParseFailureTests(unittest.TestCase):
    def test_parse_no_arguments(self) -> None:
        with self.assertRaisesRegex(ConfigParseError, "missing required argument: -p"):
            parse_args(argv())

    def test_parse_missing_team_name(self) -> None:
        with self.assertRaisesRegex(ConfigParseError, "missing required argument: -n"):
            parse_args(argv("-p", "8080"))

    def test_parse_missing_value_for_port(self) -> None:
        with self.assertRaisesRegex(ConfigParseError, "missing value for -p"):
            parse_args(argv("-p"))

    def test_parse_empty_team_name(self) -> None:
        with self.assertRaisesRegex(ConfigParseError, "team name cannot be empty"):
            parse_args(argv("-p", "8080", "-n", ""))

    def test_parse_invalid_port_zero(self) -> None:
        with self.assertRaisesRegex(ConfigParseError, "invalid value for -p: 0"):
            parse_args(argv("-p", "0", "-n", "team"))

    def test_parse_invalid_port_non_numeric(self) -> None:
        with self.assertRaisesRegex(ConfigParseError, "invalid value for -p: abc"):
            parse_args(argv("-p", "abc", "-n", "team"))

    def test_parse_unknown_flag(self) -> None:
        with self.assertRaisesRegex(ConfigParseError, "unknown argument: -z"):
            parse_args(argv("-p", "8080", "-n", "team", "-z", "1"))


class UsageTests(unittest.TestCase):
    def test_usage_string_matches_subject_format(self) -> None:
        self.assertIn("USAGE: ./zappy_ai -p port -n name -h machine", USAGE)
        self.assertIn("-p port", USAGE)
        self.assertIn("-n name", USAGE)
        self.assertIn("-h machine", USAGE)

    def test_exit_usage_code_is_84(self) -> None:
        self.assertEqual(EXIT_USAGE, 84)


if __name__ == "__main__":
    unittest.main()
