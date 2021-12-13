// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	addr = ROUNDDOWN(addr, PGSIZE);
	pte_t pte = uvpt[PGNUM(addr)];

	if ((err & FEC_WR) == 0) {
		panic("page fault: not write");
	}

	if ((err & FEC_PR) == 0) {
		panic("page fault: not mapped");
	}

	if ((pte & PTE_COW) == 0) {
		panic("page fault: not copy on write");
	}


	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	if (sys_page_alloc(0, PFTEMP, PTE_W | PTE_U | PTE_P) < 0) {
		panic("sys_page_alloc error");
	}

	memmove(PFTEMP, addr, PGSIZE);
	sys_page_map(0, PFTEMP, 0, addr, PTE_W | PTE_U | PTE_P);
	sys_page_unmap(0, PFTEMP);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	uintptr_t addr = pn * PGSIZE;
	pte_t pte = uvpt[pn];
	int perm = pte & PTE_SYSCALL;

	if (pte & PTE_W) {
		perm = perm | PTE_COW;
	}

	perm = perm & ~PTE_W;

	if (sys_page_map(0, (void *) addr, envid, (void *) addr, perm) < 0) {
		panic("sys_page_map error");
	}

	if (perm & PTE_COW) {
		if (sys_page_map(envid, (void *) addr, 0, (void *) addr, perm) <
		    0) {
			panic("sys_page_map error");
		}
	}

	return 0;
}

static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	int r;

	if (perm & PTE_W) {
		if ((r = sys_page_alloc(dstenv, va, perm)) < 0)
			panic("sys_page_alloc: %e", r);
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, perm)) < 0)
			panic("sys_page_map: %e", r);
		memmove(UTEMP, va, PGSIZE);
		if ((r = sys_page_unmap(0, UTEMP)) < 0)
			panic("sys_page_unmap: %e", r);
	} else {
		if ((r = sys_page_map(0, va, dstenv, va, perm)) < 0)
			panic("sys_page_map: %e", r);
	}
}

static envid_t
fork_v0(void)
{
	pte_t pte;


	envid_t envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);

	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}


	for (uintptr_t addr = 0; addr < UTOP; addr += PGSIZE) {
		if (uvpd[PDX(addr)] & PTE_P) {
			pte = uvpt[PGNUM(addr)];

			if (pte & PTE_P)
				dup_or_share(envid,
				             (void *) addr,
				             pte & PTE_SYSCALL);
		}
	}

	if (sys_env_set_status(envid, ENV_RUNNABLE) < 0) {
		panic("sys_env_set_status");
	}


	return envid;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	envid_t envid;
	pte_t pte;

	// 1
	set_pgfault_handler(&pgfault);

	// 2
	envid = sys_exofork();

	if (envid < 0) {
		panic("sys exofork error");
	}

	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	for (uintptr_t addr = 0; addr < UTOP; addr += PGSIZE) {
		if (uvpd[PDX(addr)] & PTE_P) {
			pte = uvpt[PGNUM(addr)];

			if (addr == (UXSTACKTOP - PGSIZE)) {
				sys_page_alloc(envid,
				               (void *) addr,
				               PTE_W | PTE_P | PTE_U);
			}

			else {
				if (pte & PTE_P)
					duppage(envid, PGNUM(addr));
			}
		}
	}

	if (sys_env_set_pgfault_upcall(envid, thisenv->env_pgfault_upcall) < 0) {
		panic("sys_env_set_pgfault_upcall error");
	}

	// 5
	if (sys_env_set_status(envid, ENV_RUNNABLE) < 0) {
		panic("sys_env_set_status error");
	}

	return envid;
}


// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
