use std::fmt;

pub const EXIT_USAGE: i32 = 84;

pub const USAGE: &str = "\
USAGE: ./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq\n\
\n\
option\t\tdescription\n\
-p port\t\tport number\n\
-x width\t\twidth of the world\n\
-y height\theight of the world\n\
-n name1 name2 ...\tname of the team\n\
-c clientsNb\tnumber of authorized clients per team\n\
-f freq\t\treciprocal of time unit for execution of actions\n";

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ServerConfig {
    pub port: u16,
    pub width: usize,
    pub height: usize,
    pub teams: Vec<String>,
    pub clients_per_team: usize,
    pub frequency: u32,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ConfigError {
    MissingValue { flag: &'static str },
    UnknownFlag(String),
    MissingFlag(&'static str),
    InvalidValue { flag: &'static str, value: String },
    NoTeams,
    EmptyTeamName,
}

impl fmt::Display for ConfigError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::MissingValue { flag } => write!(f, "missing value for {flag}"),
            Self::UnknownFlag(flag) => write!(f, "unknown argument: {flag}"),
            Self::MissingFlag(flag) => write!(f, "missing required argument: {flag}"),
            Self::InvalidValue { flag, value } => {
                write!(f, "invalid value for {flag}: {value}")
            }
            Self::NoTeams => write!(f, "at least one team name is required after -n"),
            Self::EmptyTeamName => write!(f, "team name cannot be empty"),
        }
    }
}

impl std::error::Error for ConfigError {}

pub fn parse_args(args: impl IntoIterator<Item = String>) -> Result<ServerConfig, ConfigError> {
    let args: Vec<String> = args.into_iter().collect();
    let mut port: Option<u16> = None;
    let mut width: Option<usize> = None;
    let mut height: Option<usize> = None;
    let mut teams: Vec<String> = Vec::new();
    let mut clients_per_team: Option<usize> = None;
    let mut frequency: Option<u32> = None;

    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "-p" => {
                port = Some(parse_port(next_value(&args, &mut i, "-p")?)?);
            }
            "-x" => {
                width = Some(parse_positive_usize(
                    next_value(&args, &mut i, "-x")?,
                    "-x",
                )?);
            }
            "-y" => {
                height = Some(parse_positive_usize(
                    next_value(&args, &mut i, "-y")?,
                    "-y",
                )?);
            }
            "-n" => {
                i += 1;
                let mut found = false;
                while i < args.len() && !args[i].starts_with('-') {
                    if args[i].is_empty() {
                        return Err(ConfigError::EmptyTeamName);
                    }
                    teams.push(args[i].clone());
                    found = true;
                    i += 1;
                }
                if !found {
                    return Err(ConfigError::NoTeams);
                }
                continue;
            }
            "-c" => {
                clients_per_team = Some(parse_positive_usize(
                    next_value(&args, &mut i, "-c")?,
                    "-c",
                )?);
            }
            "-f" => {
                frequency = Some(parse_positive_u32(next_value(&args, &mut i, "-f")?, "-f")?);
            }
            "--help" | "-h" => {
                return Err(ConfigError::UnknownFlag(args[i].clone()));
            }
            other => return Err(ConfigError::UnknownFlag(other.to_string())),
        }
        i += 1;
    }

    Ok(ServerConfig {
        port: port.ok_or(ConfigError::MissingFlag("-p"))?,
        width: width.ok_or(ConfigError::MissingFlag("-x"))?,
        height: height.ok_or(ConfigError::MissingFlag("-y"))?,
        teams: if teams.is_empty() {
            return Err(ConfigError::MissingFlag("-n"));
        } else {
            teams
        },
        clients_per_team: clients_per_team.ok_or(ConfigError::MissingFlag("-c"))?,
        frequency: frequency.ok_or(ConfigError::MissingFlag("-f"))?,
    })
}

fn next_value(
    args: &[String],
    index: &mut usize,
    flag: &'static str,
) -> Result<String, ConfigError> {
    *index += 1;
    args.get(*index)
        .cloned()
        .ok_or(ConfigError::MissingValue { flag })
}

fn parse_port(value: String) -> Result<u16, ConfigError> {
    let port: u32 = value.parse().map_err(|_| ConfigError::InvalidValue {
        flag: "-p",
        value: value.clone(),
    })?;
    if port == 0 || port > u16::MAX as u32 {
        return Err(ConfigError::InvalidValue { flag: "-p", value });
    }
    Ok(port as u16)
}

fn parse_positive_usize(value: String, flag: &'static str) -> Result<usize, ConfigError> {
    let parsed: usize = value.parse().map_err(|_| ConfigError::InvalidValue {
        flag,
        value: value.clone(),
    })?;
    if parsed == 0 {
        return Err(ConfigError::InvalidValue { flag, value });
    }
    Ok(parsed)
}

fn parse_positive_u32(value: String, flag: &'static str) -> Result<u32, ConfigError> {
    let parsed: u32 = value.parse().map_err(|_| ConfigError::InvalidValue {
        flag,
        value: value.clone(),
    })?;
    if parsed == 0 {
        return Err(ConfigError::InvalidValue { flag, value });
    }
    Ok(parsed)
}
