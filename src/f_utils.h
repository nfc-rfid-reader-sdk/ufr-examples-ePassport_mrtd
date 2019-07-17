/*
 * f_utils.h
 */

#ifndef F_UTILS_H_
#define F_UTILS_H_

void mrtd_fileread_write_image_to_file(const uint8_t *file_content, const int file_size, const char *filename);
void mrtd_fileread_get_datagroup_name(const uint8_t dg, char *name);
void mrtd_fileread_decode_ef_com(const uint8_t *file_content, const int file_size, uint8_t *datagroups, int *numdatagroups);

#endif /* F_UTILS_H_ */
