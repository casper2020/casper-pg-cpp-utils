# casper-pg-cpp-utils
C++ PostgreSQL extension module - utility functions

# Dependecies

```sh
ICU4C
postgresql-dev
JsonCpp
OpenSSL
```

# Build

```sh
make clean && make
```

# Installation
```sh
make install
```

# SQL - INSTALL
```sql
CREATE TYPE pg_cpp_utils_version_record AS (version text);
CREATE TYPE pg_cpp_utils_info_record AS (version text, target text, date text, repo text, dependencies text);
CREATE TYPE pg_cpp_utils_jwt_record AS (jwt text);
CREATE TYPE pg_cpp_utils_jwt_slashy_record AS (link text);
CREATE TYPE pg_cpp_utils_hash_record AS (long_hash text, short_hash text);
CREATE TYPE pg_cpp_utils_public_link_record AS (url text);
CREATE TYPE pg_cpp_utils_number_spellout_record AS (spellout text);
CREATE TYPE pg_cpp_utils_format_number_record AS (formatted text);
CREATE TYPE pg_cpp_utils_format_message_record AS (formatted text);

CREATE OR REPLACE FUNCTION pg_cpp_utils_version (  
) RETURNS pg_cpp_utils_version_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_version' LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION pg_cpp_utils_info (  
) RETURNS pg_cpp_utils_info_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_info' LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION pg_cpp_utils_make_jwt (
  a_payload text,
  a_duration integer,
  a_pkey_uri text
) RETURNS pg_cpp_utils_jwt_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_make_jwt' LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION pg_cpp_utils_make_slashy_jwt_link (
  a_base_url text,
  a_jwt text
) RETURNS pg_cpp_utils_jwt_slashy_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_make_slashy_jwt_link' LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION pg_cpp_utils_invoice_hash (
  a_pem_uri text,
  a_payload text
) RETURNS pg_cpp_utils_hash_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_invoice_hash' LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION pg_cpp_utils_public_link (
  a_base_url text,
  a_company_id float8,
  a_entity_type text,
  a_entity_id float8,
  a_key text,
  a_iv text
) RETURNS pg_cpp_utils_public_link_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_public_link' LANGUAGE C STRICT;

DROP FUNCTION IF EXISTS pg_cpp_utils_number_spellout (varchar(5), float8);
CREATE OR REPLACE FUNCTION pg_cpp_utils_number_spellout (
  a_locale            varchar(5),
  a_payload           float8,
  a_spellout_override text default ''
) RETURNS pg_cpp_utils_number_spellout_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_number_spellout' LANGUAGE C STRICT;

DROP FUNCTION IF EXISTS pg_cpp_utils_currency_spellout (varchar(5), float8, text, text, float8, text, text, text);
CREATE OR REPLACE FUNCTION pg_cpp_utils_currency_spellout (
  a_locale            varchar(5),
  a_major           float8,
  a_major_singular    text,
  a_major_plural      text,
  a_minor           float8,
  a_minor_singular    text,
  a_minor_plural      text,
  a_format            text,
  a_spellout_override text default ''
) RETURNS pg_cpp_utils_number_spellout_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_currency_spellout' LANGUAGE C STRICT;

DROP FUNCTION IF EXISTS pg_cpp_utils_format_number(float, text);
CREATE OR REPLACE FUNCTION pg_cpp_utils_format_number (
  a_locale  varchar(5),
  a_value   float8,
  a_pattern text
) RETURNS pg_cpp_utils_format_number_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_format_number' LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION pg_cpp_utils_format_message(
  a_locale  varchar(5),
  a_format  varchar(5),
  VARIADIC a_args text[]
) RETURNS pg_cpp_utils_format_message_record AS '$libdir/pg-cpp-utils.so', 'pg_cpp_utils_format_message' LANGUAGE C STRICT;

```

# Notes:

Using [ICU4C](http://site.icu-project.org)

* a_format: [RuleBasedNumberFormat](see http://userguide.icu-project.org/formatparse/numbers#TOC-RuleBasedNumberFormat)
* a_spellout_override: [RuleBasedNumberFormat](see http://userguide.icu-project.org/formatparse/numbers#TOC-RuleBasedNumberFormat)

# SQL - USAGE EXAMPLES

## Get Version:
```sql
SELECT version FROM pg_cpp_utils_version();
version
---------
 x.x.xx
(1 row)
```

## Currency Spellout:
```sql
SELECT * FROM pg_cpp_utils_currency_spellout('pt_PT', 0, 'euro', 'euros', 0, 'cêntimo', 'cêntimos',
 '{3} {0, plural, =1 {{1}} other {{2}}}{4, plural, =0 {} other { e {7} {4, plural, =1 {{5}} other {{6}}}}}'
);
  spellout  
------------
 zero euros
(1 row)
```

```sql
SELECT * FROM pg_cpp_utils_currency_spellout('pt_PT', 1, 'euro', 'euros', 0, 'cêntimo', 'cêntimos',
 '{3} {0, plural, =1 {{1}} other {{2}}}{4, plural, =0 {} other { e {7} {4, plural, =1 {{5}} other {{6}}}}}'
);
 spellout
----------
 um euro
(1 row)
```

```sql
SELECT * FROM pg_cpp_utils_currency_spellout('pt_PT', 1, 'euro', 'euros', 1, 'cêntimo', 'cêntimos',
 '{3} {0, plural, =1 {{1}} other {{2}}}{4, plural, =0 {} other { e {7} {4, plural, =1 {{5}} other {{6}}}}}'
);
       spellout       
----------------------
 um euro e um cêntimo
(1 row)
```

```sql
SELECT * FROM pg_cpp_utils_currency_spellout('pt_PT', 1, 'euro', 'euros', 2, 'cêntimo', 'cêntimos',
 '{3} {0, plural, =1 {{1}} other {{2}}}{4, plural, =0 {} other { e {7} {4, plural, =1 {{5}} other {{6}}}}}'
);
        spellout         
-------------------------
 um euro e dois cêntimos
(1 row)
```

```sql
SELECT * FROM pg_cpp_utils_currency_spellout('pt_PT', 1234567, 'euro', 'euros', 89, 'cêntimo', 'cêntimos',
 '{3} {0, plural, =1 {{1}} other {{2}}}{4, plural, =0 {} other { e {7} {4, plural, =1 {{5}} other {{6}}}}}'
);
                                                 spellout                                                  
-----------------------------------------------------------------------------------------------------------
 um milhão e duzentos e trinta e quatro mil e quinhentos e sessenta e sete euros e oitenta e nove cêntimos
(1 row)
```

## Custom Currency Spellout Override:

```sql
SELECT * FROM pg_cpp_utils_currency_spellout('pt_PT', 1234567, 'euro', 'euros', 89, 'cêntimo', 'cêntimos',
 '{3} {0, plural, =1 {{1}} other {{2}}}{4, plural, =0 {} other { e {7} {4, plural, =1 {{5}} other {{6}}}}}',
'%%lenient-parse:
    &[last primary ignorable ] << '' '' << '','' << ''-'' << ''-'';
    %spellout-numbering-year:
    x.x: =#,###0.#=;
    0: =%spellout-numbering=;
    %spellout-numbering:
    0: =%spellout-cardinal-masculine=;
    %spellout-cardinal-masculine:
    -x: menos >>;
    x.x: << vírgula >>;
    0: zero;
    1: um;
    2: dois;
    3: três;
    4: quatro;
    5: cinco;
    6: seis;
    7: sete;
    8: oito;
    9: nove;
    10: dez;
    11: onze;
    12: doze;
    13: treze;
    14: catorze;
    15: quinze;
    16: dezasseis;
    17: dezassete;
    18: dezoito;
    19: dezanove;
    20: vinte[ e >>];
    30: trinta[ e >>];
    40: quarenta[ e >>];
    50: cinquenta[ e >>];
    60: sessenta[ e >>];
    70: setenta[ e >>];
    80: oitenta[ e >>];
    90: noventa[ e >>];
    100: cem;
    101: cento e >>;
    200: duzentos[ e >>];
    300: trezentos[ e >>];
    400: quatrocentos[ e >>];
    500: quinhentos[ e >>];
    600: seiscentos[ e >>];
    700: setecentos[ e >>];
    800: oitocentos[ e >>];
    900: novecentos[ e >>];
    1000: mil[, >>];
    2000: << mil[, >>];
    1000000: um milhão[, >>];
    2000000: << milhões[, >>];
    1000000000: um bilião[, >>];
    2000000000: << biliões[, >>];
    1000000000000: um trilião[, >>];
    2000000000000: << triliões[, >>];
    1000000000000000: um quatrilião[, >>];
    2000000000000000: << quatriliões[, >>];
    1000000000000000000: =#,##0=;    
'
);
                                               spellout                                                 
---------------------------------------------------------------------------------------------------------
 um milhão, duzentos e trinta e quatro mil, quinhentos e sessenta e sete euros e oitenta e nove cêntimos
(1 row)
```
## Numbers Spellout:

```sql
SELECT * FROM pg_cpp_utils_number_spellout('pt_PT', 1234567);
                               spellout                                 
---------------------------------------------------------------------------
 um milhão e duzentos e trinta e quatro mil e quinhentos e sessenta e sete
(1 row)
```

## Custom Numbers Spellout:

```sql
SELECT * FROM pg_cpp_utils_number_spellout('pt_PT', 1234567,
'%%lenient-parse:
    &[last primary ignorable ] << '' '' << '','' << ''-'' << ''-'';
    %spellout-numbering-year:
    x.x: =#,###0.#=;
    0: =%spellout-numbering=;
    %spellout-numbering:
    0: =%spellout-cardinal-masculine=;
    %spellout-cardinal-masculine:
    -x: menos >>;
    x.x: << vírgula >>;
    0: zero;
    1: um;
    2: dois;
    3: três;
    4: quatro;
    5: cinco;
    6: seis;
    7: sete;
    8: oito;
    9: nove;
    10: dez;
    11: onze;
    12: doze;
    13: treze;
    14: catorze;
    15: quinze;
    16: dezasseis;
    17: dezassete;
    18: dezoito;
    19: dezanove;
    20: vinte[ e >>];
    30: trinta[ e >>];
    40: quarenta[ e >>];
    50: cinquenta[ e >>];
    60: sessenta[ e >>];
    70: setenta[ e >>];
    80: oitenta[ e >>];
    90: noventa[ e >>];
    100: cem;
    101: cento e >>;
    200: duzentos[ e >>];
    300: trezentos[ e >>];
    400: quatrocentos[ e >>];
    500: quinhentos[ e >>];
    600: seiscentos[ e >>];
    700: setecentos[ e >>];
    800: oitocentos[ e >>];
    900: novecentos[ e >>];
    1000: mil[, >>];
    2000: << mil[, >>];
    1000000: um milhão[, >>];
    2000000: << milhões[, >>];
    1000000000: um bilião[, >>];
    2000000000: << biliões[, >>];
    1000000000000: um trilião[, >>];
    2000000000000: << triliões[, >>];
    1000000000000000: um quatrilião[, >>];
    2000000000000000: << quatriliões[, >>];
    1000000000000000000: =#,##0=;    
'
);
                               spellout                                 
-------------------------------------------------------------------------
 um milhão, duzentos e trinta e quatro mil, quinhentos e sessenta e sete
(1 row)
```

## Number Format

```sql
select * from pg_cpp_utils_format_number('pt_PT', 12345.432, '#,##0.00 €');
  formatted  
-------------
 12.345,44 €
(1 row)
```

```sql
select * from pg_cpp_utils_format_number('en_GB', 12345.432, '£ #,##0.00');
  formatted  
-------------
 £ 12,345.44
(1 row)
```

```sql
select * from pg_cpp_utils_format_number('jp_JP', 12345.1, '#,##0 ¥');
 formatted
-----------
 12,346 ¥
```

```sql
select * from pg_cpp_utils_format_number('en_US', 12345.123, '$ #,##0.00');
  formatted  
-------------
 $ 12,345.13

```

```sql
select * from pg_cpp_utils_format_number('jp_JP', 12345.1, '#,##0 Japanese Yen');
      formatted      
---------------------
 12,346 Japanese Yen
 ```

 ```sql
 select * from pg_cpp_utils_format_number('en_US', 12345.1, '#,##0.00 US dollars');
     formatted     
-------------------
 12,346.10 US dollars
 ```

 ```sql
 SELECT * FROM pg_cpp_utils_format_message('en_US', 'A={0}, B={1}, C={2}', 'a', 'b', 'c');
 ```
