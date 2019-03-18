// Assume a set of `tests` random hashes of `hashbytes` bits each are
// generated, as well as a set of `extras`. What is the probability
// that there is at least one collision between two test hashes or
// between a test hash and an extra hash? (Collisions between two
// extras do not count).

class Birthday {
 public:
  static double collisionProbability(unsigned hashbits, double tests, double extras);
};
