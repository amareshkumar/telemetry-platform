# Stub GetGit.cmake for monorepo migration
function(thub_get_git_info GIT_TAG_VAR GIT_SHA_VAR)
    set(${GIT_TAG_VAR} "v1.0.0" PARENT_SCOPE)
    set(${GIT_SHA_VAR} "monorepo" PARENT_SCOPE)
endfunction()
