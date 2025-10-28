#pragma once

#define TYPE_REPLAY 0
#define TYPE_ONTIME 1

void ClearOrderRecord();
void LoadOrderRecord();
void SaveOrderRecord(char *s_data, int len);
void RecordUpdataIndex(int up_data);
void ReplayRecord(int up_data);

