"""Collision / pre-image prover for SMHasher. See collision_prover.py and README.md ("Collision prover").

Every family lives in its own module prover/fam_<name>.py and registers its attacks with
@attack(...) from prover.core; load_families() imports them all."""
import importlib
import pkgutil

from .core import (attack, Attack, Finding, Context, Native, Timeout, REGISTRY, MODELS, register_model, GOALS, GOAL_ALIAS,  # noqa: F401
                   DEFAULT_TIMEOUT, run_attack, run_worker, section, line, target_for)

_loaded = False


def load_families():
    global _loaded
    if _loaded:
        return
    for m in sorted(pkgutil.iter_modules(__path__), key=lambda m: m.name):
        if m.name.startswith("fam_"):
            importlib.import_module("%s.%s" % (__name__, m.name))
    _loaded = True
