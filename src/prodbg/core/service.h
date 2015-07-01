#pragma once

void Service_create();
void Service_destroy();

void Service_register(void* serviceFuncs, const char* identifier);
void* Service_getService(const char* identifier);
