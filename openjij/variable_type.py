import enum


class VariableType(enum.Enum):
    SPIN = frozenset({-1, 1})
    BINARY = frozenset({0, 1})


SPIN = VariableType.SPIN
BINARY = VariableType.BINARY


def cast_var_type(var_type):
    if isinstance(var_type, VariableType):
        return var_type
    elif isinstance(var_type, str):
        if var_type.upper() == 'SPIN':
            return SPIN
        elif var_type.upper() == 'BINARY':
            return BINARY
