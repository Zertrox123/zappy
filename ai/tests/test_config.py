import unittest

from config import (
    DEFAULT_HOST,
    EXIT_USAGE,
    USAGE,
    AiConfig,
    ConfigError,
    ConfigParseError,
    HelpRequested,
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

    def test_parse_team_name_with_underscores_and_digits(self) -> None:
        config = parse_args(argv("-p", "8080", "-n", "team_1"))
        self.assertEqual(config.team_name, "team_1")


class ParseFailureTests(unittest.TestCase):
    def test_parse_no_arguments(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv())
        self.assertEqual(ctx.exception.kind, ConfigError.MISSING_FLAG)
        self.assertEqual(ctx.exception.flag, "-p")

    def test_parse_missing_port(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-n", "team"))
        self.assertEqual(ctx.exception.kind, ConfigError.MISSING_FLAG)
        self.assertEqual(ctx.exception.flag, "-p")

    def test_parse_missing_team_name(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "8080"))
        self.assertEqual(ctx.exception.kind, ConfigError.MISSING_FLAG)
        self.assertEqual(ctx.exception.flag, "-n")

    def test_parse_missing_value_for_port(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p"))
        self.assertEqual(ctx.exception.kind, ConfigError.MISSING_VALUE)
        self.assertEqual(ctx.exception.flag, "-p")

    def test_parse_missing_value_for_team_name(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "8080", "-n"))
        self.assertEqual(ctx.exception.kind, ConfigError.MISSING_VALUE)
        self.assertEqual(ctx.exception.flag, "-n")

    def test_parse_missing_value_for_hostname(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "8080", "-n", "team", "-h"))
        self.assertEqual(ctx.exception.kind, ConfigError.MISSING_VALUE)
        self.assertEqual(ctx.exception.flag, "-h")

    def test_parse_empty_team_name(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "8080", "-n", ""))
        self.assertEqual(ctx.exception.kind, ConfigError.EMPTY_TEAM_NAME)

    def test_parse_invalid_port_zero(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "0", "-n", "team"))
        self.assertEqual(ctx.exception.kind, ConfigError.INVALID_VALUE)
        self.assertEqual(ctx.exception.flag, "-p")
        self.assertEqual(ctx.exception.value, "0")

    def test_parse_invalid_port_non_numeric(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "abc", "-n", "team"))
        self.assertEqual(ctx.exception.kind, ConfigError.INVALID_VALUE)
        self.assertEqual(ctx.exception.flag, "-p")
        self.assertEqual(ctx.exception.value, "abc")

    def test_parse_invalid_port_above_u16_max(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "65536", "-n", "team"))
        self.assertEqual(ctx.exception.kind, ConfigError.INVALID_VALUE)
        self.assertEqual(ctx.exception.flag, "-p")
        self.assertEqual(ctx.exception.value, "65536")

    def test_parse_invalid_hostname_empty(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "8080", "-n", "team", "-h", ""))
        self.assertEqual(ctx.exception.kind, ConfigError.INVALID_VALUE)
        self.assertEqual(ctx.exception.flag, "-h")

    def test_parse_unknown_flag(self) -> None:
        with self.assertRaises(ConfigParseError) as ctx:
            parse_args(argv("-p", "8080", "-n", "team", "-z", "1"))
        self.assertEqual(ctx.exception.kind, ConfigError.UNKNOWN_FLAG)
        self.assertEqual(ctx.exception.flag, "-z")

    def test_parse_help_flag_raises_help_requested(self) -> None:
        with self.assertRaises(HelpRequested):
            parse_args(argv("--help"))


class HelpAndUsageTests(unittest.TestCase):

    def test_usage_string_matches_subject_format(self) -> None:
        self.assertIn("USAGE: ./zappy_ai -p port -n name -h machine", USAGE)
        self.assertIn("-p port", USAGE)
        self.assertIn("-n name", USAGE)
        self.assertIn("-h machine", USAGE)

    def test_exit_usage_code_is_84(self) -> None:
        self.assertEqual(EXIT_USAGE, 84)

    def test_config_error_display_messages(self) -> None:
        self.assertEqual(
            ConfigError.MISSING_FLAG.format(flag="-p"),
            "missing required argument: -p",
        )
        self.assertEqual(
            ConfigError.MISSING_VALUE.format(flag="-n"),
            "missing value for -n",
        )
        self.assertEqual(
            ConfigError.INVALID_VALUE.format(flag="-p", value="bad"),
            "invalid value for -p: bad",
        )


if __name__ == "__main__":
    unittest.main()
