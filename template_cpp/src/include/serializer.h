#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Serialize a certain number of data one after the other.
 * e.g for ...: ptr, data_ptr1, size1, data_ptr2, size2, etc.
 *
 * @param count The number of data, not counting sizes and ptr to copy the data
 * to.
 * @param ... Specify the pointer to copy the data to and a certain number of
 * data pointers and sizes (in bytes).
 */
void serialize(int count, ...);

/**
 * @brief Deserialize a certain number of data one after the other.
 * e.g for ...: ptr, data_ptr1, size1, data_ptr2, size2, etc.
 * @param count The number of data, not counting sizes and ptr to copy the data
 * from.
 * @param ... Specify the pointer to copy the data from and a certain number of
 * data pointers and sizes (in bytes).
 */
void deserialize(int count, ...);

#ifdef __cplusplus
}
#endif