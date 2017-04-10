/******************************************************/
/*                                                    */
/* hamming.cpp - Hamming codes                        */
/*                                                    */
/******************************************************/

/* Hamming codes, as used in Propolis, are of length at least 3. Each of
 * the five layers consists of Hamming code blocks; all five layers have
 * the same configuration of blocks. To pack as few data bits as possible,
 * use at most two 5-bit blocks, followed by 3-bit blocks. 2**n-bit blocks
 * are not used. Examples:
 * 13: 13 (9); 7,6 (7); 7,3,3 (6); 5,5,3 (5)
 * 31: 31 (26); 21,10 (22); 13,12,6 (20); 7,7,7,7,3 (17); 7,6,6,6,3,3 (15);
 *   7,6,6,3,3,3,3 (14); 7,6,3,3,3,3,3,3 (13); 7,3,3,3,3,3,3,3,3 (12);
 *   5,5,3,3,3,3,3,3,3 (11)
 * 29694: 29694 (29679); ... 7425,(4)3712,(3)3711 (29597); (21)127,(429)63 (26973);
 *   ... (4242)7 (16968); (9898)3 (9898)
 */
