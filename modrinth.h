#ifndef MODRINTH_H
#define MODRINTH_H

int DownloadModFromModrinth(const char* modSlug, const char* gameDir, const char* loader, const char* mcVersion);
int InstallBoostFPSPack(const char* gameDir, const char* loader, const char* mcVersion);
int SearchModrinthProjects(const char* query, const char* loader, const char* mcVersion, char slugs[][64], char titles[][128], int maxResults);

#endif
