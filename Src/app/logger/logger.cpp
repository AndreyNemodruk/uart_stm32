#include "logger.h"

#include "../at24c32/at24c32.h"

#include <stdio.h>
#include <string.h>

static uint16_t write_index;
static uint16_t next_seq;
static uint16_t log_count;
static uint16_t first_seq;
static uint16_t last_seq;

static uint16_t seq_next(uint16_t seq)
{
    return (uint16_t)((seq + 1U) % LOGGER_SEQ_MOD);
}

static uint16_t seq_dist(uint16_t from, uint16_t to)
{
    return (uint16_t)((to + LOGGER_SEQ_MOD - from) % LOGGER_SEQ_MOD);
}

static bool parse_seq(const uint8_t *slot, uint16_t *seq)
{
    char tmp[LOGGER_SLOT_SIZE + 1];
    unsigned value = 0;

    if ((slot == NULL) || (slot[0] != '#'))
    {
        return false;
    }

    memcpy(tmp, slot, LOGGER_SLOT_SIZE);
    tmp[LOGGER_SLOT_SIZE] = 0;

    if (sscanf(tmp, "#%u", &value) != 1)
    {
        return false;
    }

    if (value >= LOGGER_SEQ_MOD)
    {
        return false;
    }

    *seq = (uint16_t)value;
    return true;
}

static void format_slot(uint8_t *slot, uint16_t seq, const char *message)
{
    memset(slot, 0, LOGGER_SLOT_SIZE);
    snprintf((char *)slot, LOGGER_SLOT_SIZE, "#%u %s", seq,
             (message != NULL) ? message : "");
}

static bool read_slot(uint16_t index, uint8_t *slot)
{
    uint16_t addr = (uint16_t)(index * LOGGER_SLOT_SIZE);
    return at24c32_read(addr, slot, LOGGER_SLOT_SIZE);
}

static bool write_slot(uint16_t index, const uint8_t *slot)
{
    uint16_t addr = (uint16_t)(index * LOGGER_SLOT_SIZE);
    return at24c32_write(addr, slot, LOGGER_SLOT_SIZE);
}

static bool seq_in_list(const uint16_t *seqs, uint16_t n, uint16_t seq)
{
    uint16_t i;
    for (i = 0; i < n; i++)
    {
        if (seqs[i] == seq)
        {
            return true;
        }
    }
    return false;
}

bool logger_init(void)
{
    uint8_t slot[LOGGER_SLOT_SIZE];
    uint16_t seqs[LOGGER_SLOT_COUNT];
    uint16_t slots[LOGGER_SLOT_COUNT];
    uint16_t n = 0;
    uint16_t i;
    uint16_t newest_seq = 0;
    uint16_t newest_slot = 0;
    uint16_t oldest_seq = 0;
    bool have_newest = false;

    write_index = 0;
    next_seq = 0;
    log_count = 0;
    first_seq = 0;
    last_seq = 0;

    for (i = 0; i < LOGGER_SLOT_COUNT; i++)
    {
        uint16_t seq;

        if (!read_slot(i, slot))
        {
            printf("[LOG] EEPROM read failed at slot %u\r\n", i);
            return false;
        }

        if (parse_seq(slot, &seq))
        {
            seqs[n] = seq;
            slots[n] = i;
            n++;
        }
    }

    if (n == 0U)
    {
        printf("[LOG] empty, count=0\r\n");
        return true;
    }

    for (i = 0; i < n; i++)
    {
        if (!seq_in_list(seqs, n, seq_next(seqs[i])))
        {
            newest_seq = seqs[i];
            newest_slot = slots[i];
            have_newest = true;
            break;
        }
    }

    if (!have_newest)
    {
        newest_seq = seqs[0];
        newest_slot = slots[0];
    }

    oldest_seq = newest_seq;
    for (i = 0; i < n; i++)
    {
        if (seq_dist(seqs[i], newest_seq) > seq_dist(oldest_seq, newest_seq))
        {
            oldest_seq = seqs[i];
        }
    }

    log_count = n;
    first_seq = oldest_seq;
    last_seq = newest_seq;
    write_index = (uint16_t)((newest_slot + 1U) % LOGGER_SLOT_COUNT);
    next_seq = seq_next(newest_seq);

    printf("[LOG] count=%u first=#%u last=#%u\r\n", log_count, first_seq, last_seq);
    return true;
}

bool logger_log(const char *message)
{
    uint8_t slot[LOGGER_SLOT_SIZE];

    format_slot(slot, next_seq, message);
    if (!write_slot(write_index, slot))
    {
        printf("[LOG] write failed\r\n");
        return false;
    }

    last_seq = next_seq;
    if (log_count == 0U)
    {
        first_seq = next_seq;
        log_count = 1;
    }
    else if (log_count < LOGGER_SLOT_COUNT)
    {
        log_count++;
    }
    else
    {
        first_seq = seq_next(first_seq);
    }

    write_index = (uint16_t)((write_index + 1U) % LOGGER_SLOT_COUNT);
    next_seq = seq_next(next_seq);
    return true;
}

static bool slot_index_for_seq(uint16_t number, uint16_t *index)
{
    uint16_t dist;
    uint16_t oldest_slot;

    if ((log_count == 0U) || (number >= LOGGER_SEQ_MOD))
    {
        return false;
    }

    dist = seq_dist(first_seq, number);
    if (dist >= log_count)
    {
        return false;
    }

    oldest_slot = (uint16_t)((write_index + LOGGER_SLOT_COUNT - log_count) % LOGGER_SLOT_COUNT);
    *index = (uint16_t)((oldest_slot + dist) % LOGGER_SLOT_COUNT);
    return true;
}

bool logger_read(uint16_t number, char *buffer)
{
    uint8_t slot[LOGGER_SLOT_SIZE];
    uint16_t index;
    uint16_t seq;

    if (!slot_index_for_seq(number, &index))
    {
        printf("[LOG] #%u not found\r\n", number);
        return false;
    }

    if (!read_slot(index, slot) || !parse_seq(slot, &seq) || (seq != number))
    {
        printf("[LOG] #%u read failed\r\n", number);
        return false;
    }

    slot[LOGGER_SLOT_SIZE - 1U] = 0;
    if (buffer != NULL)
    {
        memcpy(buffer, slot, LOGGER_SLOT_SIZE);
    }

    printf("%s\r\n", (char *)slot);
    return true;
}

bool logger_read_range(uint16_t first, uint16_t last)
{
    uint16_t seq;
    uint16_t steps = 0;

    if ((first >= LOGGER_SEQ_MOD) || (last >= LOGGER_SEQ_MOD))
    {
        return false;
    }

    seq = first;
    while (steps < LOGGER_SEQ_MOD)
    {
        logger_read(seq, NULL);
        if (seq == last)
        {
            return true;
        }
        seq = seq_next(seq);
        steps++;
    }

    return false;
}

uint16_t logger_get_count(void)
{
    return log_count;
}

uint16_t logger_get_first(void)
{
    return first_seq;
}

uint16_t logger_get_last(void)
{
    return last_seq;
}
