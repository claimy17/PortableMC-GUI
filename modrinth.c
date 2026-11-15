#include "modrinth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  #define SEP '\\'
#else
  #define POPEN popen
  #define PCLOSE pclose
  #define SEP '/'
#endif

static void get_modrinth_hosts(char* apiHost, char* cdnHost) {
    sprintf(apiHost, "api%cmodrinth%ccom/v2", '.', '.');
    sprintf(cdnHost, "cdn%cmodrinth%ccom", '.', '.');
}

static void url_encode(const char* src, char* dst, size_t maxLen) {
    const char* hex = "0123456789ABCDEF";
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j + 4 < maxLen; i++) {
        unsigned char c = (unsigned char)src[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_') {
            dst[j++] = c;
        } else if (c == ' ') {
            dst[j++] = '%'; dst[j++] = '2'; dst[j++] = '0';
        } else {
            dst[j++] = '%';
            dst[j++] = hex[(c >> 4) & 0xF];
            dst[j++] = hex[c & 0xF];
        }
    }
    dst[j] = '\0';
}

static size_t HttpGetToBuffer(const char* url, char* outBuf, size_t maxBuf) {
    char* cmd = (char*)calloc(2048, 1);
    if (!cmd) return 0;

#ifdef _WIN32
    snprintf(cmd, 2048, "curl.exe -s -L -k -A \"PortableMC-GUI/2.0\" \"%s\"", url);

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hRead = NULL, hWrite = NULL;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) { free(cmd); return 0; }
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = { 0 };
    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        free(cmd);
        return 0;
    }
    CloseHandle(hWrite);

    size_t total = 0;
    DWORD bytesRead = 0;
    while (total + 1 < maxBuf && ReadFile(hRead, outBuf + total, (DWORD)(maxBuf - 1 - total), &bytesRead, NULL) && bytesRead > 0) {
        total += bytesRead;
    }
    outBuf[total] = '\0';

    WaitForSingleObject(pi.hProcess, 10000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hRead);
    free(cmd);
    return total;
#else
    snprintf(cmd, 2048, "curl -s -L -A \"PortableMC-GUI/2.0\" \"%s\"", url);
    FILE* fp = POPEN(cmd, "r");
    free(cmd);
    if (!fp) return 0;
    size_t total = fread(outBuf, 1, maxBuf - 1, fp);
    outBuf[total] = '\0';
    PCLOSE(fp);
    return total;
#endif
}

static int HttpDownloadToFile(const char* url, const char* filePath) {
    char* cmd = (char*)calloc(2048, 1);
    if (!cmd) return 0;

#ifdef _WIN32
    snprintf(cmd, 2048, "curl.exe -s -L -k -A \"PortableMC-GUI/2.0\" \"%s\" -o \"%s\"", url, filePath);

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = { 0 };
    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        free(cmd);
        return 0;
    }
    WaitForSingleObject(pi.hProcess, 30000);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    free(cmd);
    return (exitCode == 0) ? 1 : 0;
#else
    snprintf(cmd, 2048, "curl -s -L -A \"PortableMC-GUI/2.0\" \"%s\" -o \"%s\"", url, filePath);
    int res = system(cmd);
    free(cmd);
    return (res == 0) ? 1 : 0;
#endif
}

int DownloadModFromModrinth(const char* modSlug, const char* gameDir, const char* loader, const char* mcVersion) {
    if (!modSlug || !gameDir || !loader || !mcVersion) return 0;

    char* apiHost = (char*)calloc(64, 1);
    char* cdnHost = (char*)calloc(64, 1);
    char* url = (char*)calloc(2048, 1);
    char* outPath = (char*)calloc(1024, 1);
    char* jsonBuf = (char*)calloc(262144, 1);
    if (!apiHost || !cdnHost || !url || !outPath || !jsonBuf) {
        free(apiHost); free(cdnHost); free(url); free(outPath); free(jsonBuf);
        return 0;
    }

    get_modrinth_hosts(apiHost, cdnHost);

    snprintf(url, 2048,
        "https://%s/project/%s/version?loaders=%%5B%%22%s%%22%%5D&game_versions=%%5B%%22%s%%22%%5D",
        apiHost, modSlug, loader, mcVersion);

    size_t n = HttpGetToBuffer(url, jsonBuf, 262144);
    if (n < 10 || !strstr(jsonBuf, cdnHost)) {
        memset(jsonBuf, 0, 262144);
        snprintf(url, 2048, "https://%s/project/%s/version", apiHost, modSlug);
        HttpGetToBuffer(url, jsonBuf, 262144);
    }

    char* cdnPos = strstr(jsonBuf, cdnHost);
    if (!cdnPos || (cdnPos - jsonBuf) < 8) {
        free(apiHost); free(cdnHost); free(url); free(outPath); free(jsonBuf);
        return 0;
    }
    char* urlStart = cdnPos - 8;

    char* urlEnd = strchr(urlStart, '"');
    if (!urlEnd) {
        free(apiHost); free(cdnHost); free(url); free(outPath); free(jsonBuf);
        return 0;
    }
    *urlEnd = '\0';

    char* fileName = strrchr(urlStart, '/');
    if (!fileName) {
        free(apiHost); free(cdnHost); free(url); free(outPath); free(jsonBuf);
        return 0;
    }
    fileName++;

#ifdef _WIN32
    snprintf(outPath, 1024, "%s\\mods", gameDir);
    CreateDirectoryA(outPath, NULL);
#else
    snprintf(outPath, 1024, "mkdir -p \"%s/mods\"", gameDir);
    system(outPath);
#endif

    snprintf(outPath, 1024, "%s%cmods%c%s", gameDir, SEP, SEP, fileName);
    int ok = HttpDownloadToFile(urlStart, outPath);

    free(apiHost);
    free(cdnHost);
    free(url);
    free(outPath);
    free(jsonBuf);
    return ok;
}
int InstallBoostFPSPack(const char* gameDir, const char* loader, const char* mcVersion) {
    const char* mods[] = { "fabric-api", "sodium", "lithium", "ferrite-core", "entityculling" };
    int count = 0;
    for (int i = 0; i < 5; i++) {
        if (i == 0 && strcmp(loader, "fabric") != 0 && strcmp(loader, "quilt") != 0) continue;
        if (DownloadModFromModrinth(mods[i], gameDir, loader, mcVersion)) {
            count++;
        }
    }
    return count;
}

int SearchModrinthProjects(const char* query, const char* loader, const char* mcVersion, char slugs[][64], char titles[][128], int maxResults) {
    char* apiHost = (char*)calloc(64, 1);
    char* cdnHost = (char*)calloc(64, 1);
    char* encoded = (char*)calloc(512, 1);
    char* url = (char*)calloc(2048, 1);
    char* jsonBuf = (char*)calloc(262144, 1);
    if (!apiHost || !cdnHost || !encoded || !url || !jsonBuf) {
        free(apiHost); free(cdnHost); free(encoded); free(url); free(jsonBuf);
        return 0;
    }

    get_modrinth_hosts(apiHost, cdnHost);
    url_encode(query, encoded, 512);

    snprintf(url, 2048,
        "https://%s/search?query=%s&limit=%d",
        apiHost, encoded, maxResults);

    size_t bytes = HttpGetToBuffer(url, jsonBuf, 262144);

    if (bytes == 0 || !strstr(jsonBuf, "\"slug\":\"")) {
#ifdef _WIN32
        char* dbg = (char*)calloc(1024, 1);
        if (dbg) {
            snprintf(dbg, 1024, "URL:\n%s\n\nBytes ricevuti: %zu\nRisposta:\n%.250s", url, bytes, jsonBuf);
            MessageBoxA(NULL, dbg, "Diagnostica Modrinth", MB_OK);
            free(dbg);
        }
#endif
        free(apiHost); free(cdnHost); free(encoded); free(url); free(jsonBuf);
        return 0;
    }

    int found = 0;
    char* ptr = jsonBuf;
    while (found < maxResults && (ptr = strstr(ptr, "\"slug\":\"")) != NULL) {
        ptr += 8;
        char* endSlug = strchr(ptr, '"');
        if (!endSlug) break;
        size_t sLen = (size_t)(endSlug - ptr);
        if (sLen >= 64) sLen = 63;
        strncpy(slugs[found], ptr, sLen);
        slugs[found][sLen] = '\0';

        char* tPtr = strstr(endSlug, "\"title\":\"");
        if (tPtr) {
            tPtr += 9;
            char* endTitle = strchr(tPtr, '"');
            if (endTitle) {
                size_t tLen = (size_t)(endTitle - tPtr);
                if (tLen >= 128) tLen = 127;
                strncpy(titles[found], tPtr, tLen);
                titles[found][tLen] = '\0';
                found++;
                ptr = endTitle;
                continue;
            }
        }
        ptr = endSlug;
    }

    free(apiHost);
    free(cdnHost);
    free(encoded);
    free(url);
    free(jsonBuf);
    return found;
}
