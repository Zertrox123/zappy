class CommandBufferFullError(RuntimeError):
    pass


class CommandInFlightError(RuntimeError):
    pass


class CommandQueue:
    MAX_PENDING = 10

    def __init__(self) -> None:
        self._pending = 0
        self._in_flight = False

    @property
    def pending(self) -> int:
        return self._pending

    @property
    def in_flight(self) -> bool:
        return self._in_flight

    def acquire(self) -> None:
        if self._in_flight:
            raise CommandInFlightError("command already in flight")
        if self._pending >= self.MAX_PENDING:
            raise CommandBufferFullError("command buffer full")
        self._in_flight = True
        self._pending += 1

    def complete(self) -> None:
        if not self._in_flight:
            raise CommandInFlightError("no command in flight")
        self._in_flight = False
        self._pending = max(0, self._pending - 1)
