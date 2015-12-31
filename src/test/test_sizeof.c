int main() {
	int length = sizeof (int);
	char c1[ length ] ;
	char c2[ sizeof (int) ] ;
	return sizeof c1;
}
