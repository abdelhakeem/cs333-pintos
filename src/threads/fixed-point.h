/* Fixed-Point constants. */
#define FIXED_P_FRAQ_BITS 14
#define FIXED_P_F (1 << FIXED_P_FRAQ_BITS)

/* Fixed-Point type. */
typedef int fixed_p;

fixed_p int_to_fixed_p (int);
int fixed_p_to_int (fixed_p);

fixed_p fixed_p_multiply (fixed_p, fixed_p);
fixed_p fixed_p_divde (fixed_p, fixed_p);


