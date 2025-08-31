typedef struct rolling_hash
{
    int max_size;
    long long base;
    long long mod;
    long long current_hash;
    long long base_power;
} rolling_hash_t;

void rolling_hash_init(rolling_hash_t *rh, int max_size, long long base, long long mod)
{
    rh->max_size = max_size;
    rh->base = base;
    rh->mod = mod;
    rh->current_hash = 0;
    rh->base_power = 1;
    for (int i = 0; i < max_size - 1; i++)
    {
        rh->base_power = (rh->base_power * base) % mod;
    }
}

void rolling_hash_reset(rolling_hash_t *rh)
{
    rh->current_hash = 0;
    rh->base_power = 1;
}

void rolling_hash_push(rolling_hash_t *rh, char new_char)
{
    rh->current_hash = (rh->current_hash * rh->base + new_char) % rh->mod;
}

