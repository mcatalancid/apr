/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2002 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include "threadproc.h"
#include "fileio.h"
#include "apr_strings.h"
#include "apr_portable.h"

#include <nks/vm.h>

apr_status_t apr_netware_proc_cleanup(void *theproc)
{
    apr_proc_t *proc = theproc;

	NXVmDestroy(proc->pid);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_create(apr_procattr_t **new,apr_pool_t *cont)
{
    (*new) = (apr_procattr_t *)apr_pcalloc(cont, sizeof(apr_procattr_t));

    if ((*new) == NULL) {
        return APR_ENOMEM;
    }
    (*new)->cntxt = cont;
    (*new)->cmdtype = APR_PROGRAM;
    return APR_SUCCESS;

}

APR_DECLARE(apr_status_t) apr_procattr_io_set(apr_procattr_t *attr, apr_int32_t in, 
                                 apr_int32_t out, apr_int32_t err)
{
    apr_status_t status;
    if (in != 0) {
        if ((status = apr_file_pipe_create(&attr->child_in, &attr->parent_in, 
                                   attr->cntxt)) != APR_SUCCESS) {
            return status;
        }
        switch (in) {
        case APR_FULL_BLOCK:
            break;
        case APR_PARENT_BLOCK:
            apr_file_pipe_timeout_set(attr->child_in, 0);
            break;
        case APR_CHILD_BLOCK:
            apr_file_pipe_timeout_set(attr->parent_in, 0);
            break;
        default:
            apr_file_pipe_timeout_set(attr->child_in, 0);
            apr_file_pipe_timeout_set(attr->parent_in, 0);
        }
    } 
    if (out) {
        if ((status = apr_file_pipe_create(&attr->parent_out, &attr->child_out, 
                                   attr->cntxt)) != APR_SUCCESS) {
            return status;
        }
        switch (out) {
        case APR_FULL_BLOCK:
            break;
        case APR_PARENT_BLOCK:
            apr_file_pipe_timeout_set(attr->child_out, 0);
            break;
        case APR_CHILD_BLOCK:
            apr_file_pipe_timeout_set(attr->parent_out, 0);
            break;
        default:
            apr_file_pipe_timeout_set(attr->child_out, 0);
            apr_file_pipe_timeout_set(attr->parent_out, 0);
        }
    } 
    if (err) {
        if ((status = apr_file_pipe_create(&attr->parent_err, &attr->child_err, 
                                   attr->cntxt)) != APR_SUCCESS) {
            return status;
        }
        switch (err) {
        case APR_FULL_BLOCK:
            break;
        case APR_PARENT_BLOCK:
            apr_file_pipe_timeout_set(attr->child_err, 0);
            break;
        case APR_CHILD_BLOCK:
            apr_file_pipe_timeout_set(attr->parent_err, 0);
            break;
        default:
            apr_file_pipe_timeout_set(attr->child_err, 0);
            apr_file_pipe_timeout_set(attr->parent_err, 0);
        }
    } 
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_procattr_child_in_set(apr_procattr_t *attr, apr_file_t *child_in,
                                   apr_file_t *parent_in)
{
    if (attr->child_in == NULL && attr->parent_in == NULL)
        apr_file_pipe_create(&attr->child_in, &attr->parent_in, attr->cntxt);

    if (child_in != NULL)
        apr_file_dup2(attr->child_in, child_in, attr->cntxt);

    if (parent_in != NULL)
        apr_file_dup2(attr->parent_in, parent_in, attr->cntxt);

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_procattr_child_out_set(apr_procattr_t *attr, apr_file_t *child_out,
                                    apr_file_t *parent_out)
{
    if (attr->child_out == NULL && attr->parent_out == NULL)
        apr_file_pipe_create(&attr->child_out, &attr->parent_out, attr->cntxt);

    if (child_out != NULL)
        apr_file_dup2(attr->child_out, child_out, attr->cntxt);

    if (parent_out != NULL)
        apr_file_dup2(attr->parent_out, parent_out, attr->cntxt);

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_procattr_child_err_set(apr_procattr_t *attr, apr_file_t *child_err,
                                   apr_file_t *parent_err)
{
    if (attr->child_err == NULL && attr->parent_err == NULL)
        apr_file_pipe_create(&attr->child_err, &attr->parent_err, attr->cntxt);

    if (child_err != NULL)
        apr_file_dup2(attr->child_err, child_err, attr->cntxt);

    if (parent_err != NULL)
        apr_file_dup2(attr->parent_err, parent_err, attr->cntxt);

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_procattr_dir_set(apr_procattr_t *attr, 
                               const char *dir) 
{
    attr->currdir = apr_pstrdup(attr->cntxt, dir);
    if (attr->currdir) {
        return APR_SUCCESS;
    }
    return APR_ENOMEM;
}

APR_DECLARE(apr_status_t) apr_procattr_cmdtype_set(apr_procattr_t *attr,
                                     apr_cmdtype_e cmd) 
{
    if (cmd != APR_PROGRAM)
        return APR_ENOTIMPL;
    attr->cmdtype = cmd;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_detach_set(apr_procattr_t *attr, apr_int32_t detach) 
{
    attr->detached = detach;
    return APR_SUCCESS;
}

#if APR_HAS_FORK
APR_DECLARE(apr_status_t) apr_proc_fork(apr_proc_t *proc, apr_pool_t *cont)
{
    int pid;
    
    if ((pid = fork()) < 0) {
        return errno;
    }
    else if (pid == 0) {
        proc->pid = pid;
        proc->in = NULL; 
        proc->out = NULL; 
        proc->err = NULL; 
        return APR_INCHILD;
    }
    proc->pid = pid;
    proc->in = NULL; 
    proc->out = NULL; 
    proc->err = NULL; 
    return APR_INPARENT;
}
#endif

static apr_status_t limit_proc(apr_procattr_t *attr)
{
#if APR_HAVE_STRUCT_RLIMIT && APR_HAVE_SETRLIMIT
#ifdef RLIMIT_CPU
    if (attr->limit_cpu != NULL) {
        if ((setrlimit(RLIMIT_CPU, attr->limit_cpu)) != 0) {
            return errno;
        }
    }
#endif
#ifdef RLIMIT_NPROC
    if (attr->limit_nproc != NULL) {
        if ((setrlimit(RLIMIT_NPROC, attr->limit_nproc)) != 0) {
            return errno;
        }
    }
#endif
#if defined(RLIMIT_AS)
    if (attr->limit_mem != NULL) {
        if ((setrlimit(RLIMIT_AS, attr->limit_mem)) != 0) {
            return errno;
        }
    }
#elif defined(RLIMIT_DATA)
    if (attr->limit_mem != NULL) {
        if ((setrlimit(RLIMIT_DATA, attr->limit_mem)) != 0) {
            return errno;
        }
    }
#elif defined(RLIMIT_VMEM)
    if (attr->limit_mem != NULL) {
        if ((setrlimit(RLIMIT_VMEM, attr->limit_mem)) != 0) {
            return errno;
        }
    }
#endif
#else
    /*
     * Maybe make a note in error_log that setrlimit isn't supported??
     */

#endif
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_create(apr_proc_t *newproc,
									const char *progname, 
									const char * const *args, 
									const char * const *env,
                              		apr_procattr_t *attr, 
                              		apr_pool_t *cont)
{
    int i, envCount;
    const char **newargs;
    char **newenv;
    NXVmId_t newVM;
    unsigned long flags = 0;

    NXNameSpec_t nameSpec;
    NXExecEnvSpec_t envSpec;

    /* Set up the info for the NLM to be started */
    nameSpec.ssType = NX_OBJ_FILE;
    nameSpec.ssPathCtx = NULL;
    nameSpec.ssPath = (void*)progname;

    /* Count how many arguments there are and assign them 
        to the environent */
    for (i=0;args && args[i];i++);
    envSpec.esArgc = i;
    envSpec.esArgv = (void**)args;

    /* Count how many environment variables there are in the
        system, add any new environment variables and place
        them in the environment. */
    for (i=0;env && env[i];i++);
    envCount = NXGetEnvCount();
    if ((envCount + i) > 0) {
        newenv = (char **) NXMemAlloc(sizeof(char *) * (envCount+i+1), 0);
        if (!newenv)
            return APR_ENOMEM;
        NXCopyEnv(newenv, envCount);
        for (i=0;env && env[i];i++) {
            newenv[envCount+i-1] = (char*)env[i];
        }
        newenv[envCount+i] = NULL;

        envSpec.esEnv = (void**)newenv;
    }
    else
        envSpec.esEnv = NULL;

    envSpec.esStdin.ssType = NX_OBJ_FIFO;
    envSpec.esStdin.ssHandle = -1;
    envSpec.esStdin.ssPathCtx = 0;
    if (attr->child_in) {
        apr_pool_cleanup_kill(apr_file_pool_get(attr->child_in), 
                              attr->child_in, apr_netware_pipe_cleanup);
        envSpec.esStdin.ssPath = attr->child_in->fname;
        apr_file_close(attr->child_in);
        if (attr->parent_in) {
            apr_file_close(attr->parent_in);
        }
    }
    else if (attr->parent_in) {
        envSpec.esStdin.ssPath = attr->parent_in->fname;
        apr_file_close(attr->parent_in);
    }
    else {
        envSpec.esStdin.ssPath = NULL;
    }

    envSpec.esStdout.ssType = NX_OBJ_FIFO;
    envSpec.esStdout.ssHandle = -1;
    envSpec.esStdout.ssPathCtx = 0;
    if (attr->child_out) {
        apr_pool_cleanup_kill(apr_file_pool_get(attr->child_out),
                              attr->child_out, apr_netware_pipe_cleanup);
        envSpec.esStdout.ssPath = attr->child_out->fname;
        apr_file_close(attr->child_out);
        if (attr->parent_out) {
            apr_file_close(attr->parent_out);
        }
    }
    else if (attr->parent_out) {
        envSpec.esStdout.ssPath = attr->parent_out->fname;
        apr_file_close(attr->parent_out);
    }
    else {
        envSpec.esStdout.ssPath = NULL;
    }

    envSpec.esStderr.ssType = NX_OBJ_FIFO;
    envSpec.esStderr.ssHandle = -1;
    envSpec.esStderr.ssPathCtx = 0;
    if (attr->child_err) {
        apr_pool_cleanup_kill(apr_file_pool_get(attr->child_err),
                              attr->child_err, apr_netware_pipe_cleanup);
        envSpec.esStderr.ssPath = attr->child_err->fname;
        apr_file_close(attr->child_err);
        if (attr->parent_err) {
            apr_file_close(attr->parent_err);
        }
    }
    else if (attr->parent_err) {
        envSpec.esStderr.ssPath = attr->parent_err->fname;
        apr_file_close(attr->parent_err);
    }
    else {
        envSpec.esStderr.ssPath = NULL;
    }

    if (attr->detached) {
        flags = NX_VM_CREATE_DETACHED;
    }
    
    newproc->in = attr->parent_in;
    newproc->err = attr->parent_err;
    newproc->out = attr->parent_out;
    if (NXVmSpawn(&nameSpec, &envSpec, flags, &newVM) != 0) {
        return errno;
    }
    else { 
        newproc->pid = newVM;
        if (attr->parent_out) {
    	    attr->parent_out->filedes = pipe_open(attr->parent_out->fname, O_WRONLY);
        }
        if (attr->parent_in) {
    	    attr->parent_in->filedes = pipe_open(attr->parent_in->fname, O_RDONLY);
        }
        if (attr->parent_err) {
    	    attr->parent_err->filedes = pipe_open(attr->parent_err->fname, O_RDONLY);
        }

        apr_pool_cleanup_register(cont, (void *)newproc, apr_netware_proc_cleanup,
                         apr_pool_cleanup_null);
    }
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_wait_all_procs(apr_proc_t *proc,
                                                  int *exitcode,
                                                  apr_exit_why_e *exitwhy,
                                                  apr_wait_how_e waithow,
                                                  apr_pool_t *p)
{
#if 0
    int waitpid_options = WUNTRACED;

    if (waithow != APR_WAIT) {
        waitpid_options |= WNOHANG;
    }

    if ((proc->pid = waitpid(-1, status, waitpid_options)) > 0) {
        return APR_CHILD_DONE;
    }
    else if (proc->pid == 0) {
        return APR_CHILD_NOTDONE;
    }
    return errno;
#else
    return APR_ENOTIMPL;
#endif
} 

APR_DECLARE(apr_status_t) apr_proc_wait(apr_proc_t *proc,
                                        int *exitcode, apr_exit_why_e *exitwhy,
                                        apr_wait_how_e waithow)
{
#if 0
    pid_t status;

    if (waithow == APR_WAIT) {
        if ((status = waitpid(proc->pid, NULL, WUNTRACED)) > 0) {
            return APR_CHILD_DONE;
        }
        else if (status == 0) {
            return APR_CHILD_NOTDONE;
        }
        return errno;
    }
    if ((status = waitpid(proc->pid, NULL, WUNTRACED | WNOHANG)) > 0) {
        return APR_CHILD_DONE;
    }
    else if (status == 0) {
        return APR_CHILD_NOTDONE;
    }
    return errno;
#else
    return APR_ENOTIMPL;
#endif
} 

APR_DECLARE(apr_status_t) apr_procattr_limit_set(apr_procattr_t *attr, apr_int32_t what, 
                          struct rlimit *limit)
{
    switch(what) {
        case APR_LIMIT_CPU:
#ifdef RLIMIT_CPU
            attr->limit_cpu = limit;
            break;
#else
            return APR_ENOTIMPL;
#endif
        case APR_LIMIT_MEM:
#if defined (RLIMIT_DATA) || defined (RLIMIT_VMEM) || defined(RLIMIT_AS)
            attr->limit_mem = limit;
            break;
#else
            return APR_ENOTIMPL;
#endif
        case APR_LIMIT_NPROC:
#ifdef RLIMIT_NPROC
            attr->limit_nproc = limit;
            break;
#else
            return APR_ENOTIMPL;
#endif
    }
    return APR_SUCCESS;
}  

