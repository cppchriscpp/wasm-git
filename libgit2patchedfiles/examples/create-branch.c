/*
 * libgit2 "create_branch" example - shows how to push to remote
 *
 * Written by the libgit2 contributors
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include "common.h"

/**
 * This example demonstrates the libgit2 branch creation API. It is not a built-in
 * git command, so some liberties have been taken.
 *
 */

static void print_usage(void)
{
		fprintf(stderr, "usage: create_branch <branch>\n");
		exit(1);
}


/** Entry point for this command */
int lg2_create_branch(git_repository *repo, int argc, char **argv) {
	git_repository_state_t state;
	git_reference *branch_out = NULL;
	git_commit *commit = NULL;
	git_oid oid_parent_commit;
	git_reference *head = NULL;
	int err = 0;
	git_remote* remote = NULL;


	char *origin_name;
	char *ref_spec;

	/* Validate args */
	if (argc <= 1) {
		print_usage();
	}

	/** Make sure we're not about to checkout while something else is going on */
	state = git_repository_state(repo);
	if (state != GIT_REPOSITORY_STATE_NONE) {
		fprintf(stderr, "repository is in unexpected state %d\n", state);
		exit(1);
	}

	// Figure out origin name
	origin_name = malloc(strlen("origin/") + strlen(argv[1]));
	if (origin_name == NULL) {
		printf("Branch creation failed due to memory issues\n");
		exit(1);
	}
	strcpy(origin_name, "origin/");
	strcat(origin_name, argv[1]);

	check_lg2(git_reference_name_to_id(&oid_parent_commit, repo, "HEAD"), "Could not get HEAD commit from repo", NULL);
	check_lg2(git_commit_lookup(&commit, repo, &oid_parent_commit), "Could not look up parent commit for HEAD", NULL);

	check_lg2(git_branch_create(&branch_out, repo, argv[1], commit, 0), "Failed creating branch", NULL);
	
	check_lg2(git_reference_symbolic_create(&head, repo, "HEAD", git_reference_name(branch_out), 1, "Switching to new branch"), "Failed pointing HEAD at new branch", NULL);

     // get the remote.
     check_lg2(git_remote_lookup( &remote, repo, "origin" ), "Failed looking up remote", NULL);

     // connect to remote
     check_lg2(git_remote_connect( remote, GIT_DIRECTION_PUSH, NULL, NULL, NULL ), "Failed connecting to remote", NULL);

	// Build the refspec
	ref_spec = malloc(strlen("refs/heads/:refs/heads/") + (strlen(argv[1])*2));
	if (ref_spec == NULL) {
		printf("Branch creation failed due to memory issues later on\n");
		exit(1);
	}
	strcpy(ref_spec, "refs/heads/");
	strcat(ref_spec, argv[1]);
	strcat(ref_spec, "refs/heads/");
	strcat(ref_spec, argv[1]);


     // add a push refspec
    check_lg2(git_remote_add_push(repo, "origin", ref_spec), "Failed adding remote repo", NULL);
	check_lg2(git_remote_add_fetch(repo, "origin", ref_spec), "Failed adding remote repo", NULL);

	printf("Branch created\n");


	git_reference_free(branch_out);
	git_reference_free(head);
	free(origin_name);
	git_remote_free(remote);
	return err;
}
