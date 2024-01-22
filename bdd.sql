--
-- PostgreSQL database dump
--

-- Dumped from database version 16.1
-- Dumped by pg_dump version 16.1

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: files; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.files (
    id integer NOT NULL,
    file_name character varying(255) NOT NULL,
    id_patient integer
);


ALTER TABLE public.files OWNER TO postgres;

--
-- Name: files_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.files_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.files_id_seq OWNER TO postgres;

--
-- Name: files_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.files_id_seq OWNED BY public.files.id;


--
-- Name: historique_sante; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.historique_sante (
    id_historique integer NOT NULL,
    date date,
    type character varying(255),
    valeur character varying(255),
    id_utilisateur integer
);


ALTER TABLE public.historique_sante OWNER TO postgres;

--
-- Name: historique_sante_id_historique_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.historique_sante_id_historique_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.historique_sante_id_historique_seq OWNER TO postgres;

--
-- Name: historique_sante_id_historique_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.historique_sante_id_historique_seq OWNED BY public.historique_sante.id_historique;


--
-- Name: medicament; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.medicament (
    id_medicament integer NOT NULL,
    nom character varying(255),
    description text,
    dosage character varying(255),
    id_patient integer
);


ALTER TABLE public.medicament OWNER TO postgres;

--
-- Name: medicament_id_medicament_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.medicament_id_medicament_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.medicament_id_medicament_seq OWNER TO postgres;

--
-- Name: medicament_id_medicament_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.medicament_id_medicament_seq OWNED BY public.medicament.id_medicament;


--
-- Name: notification; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.notification (
    id_rappel integer NOT NULL,
    id_medicament integer NOT NULL
);


ALTER TABLE public.notification OWNER TO postgres;

--
-- Name: rappel; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.rappel (
    id_rappel integer NOT NULL,
    rendez_vous integer,
    conseil text,
    meteo text,
    message text,
    id_patient integer
);


ALTER TABLE public.rappel OWNER TO postgres;

--
-- Name: rappel_id_rappel_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.rappel_id_rappel_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.rappel_id_rappel_seq OWNER TO postgres;

--
-- Name: rappel_id_rappel_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.rappel_id_rappel_seq OWNED BY public.rappel.id_rappel;


--
-- Name: rendez_vous; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.rendez_vous (
    id_rendezvous integer NOT NULL,
    date date,
    heure time without time zone,
    lieu character varying(255),
    description text,
    id_patient integer,
    id_medecin integer
);


ALTER TABLE public.rendez_vous OWNER TO postgres;

--
-- Name: rendez_vous_id_rendezvous_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.rendez_vous_id_rendezvous_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.rendez_vous_id_rendezvous_seq OWNER TO postgres;

--
-- Name: rendez_vous_id_rendezvous_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.rendez_vous_id_rendezvous_seq OWNED BY public.rendez_vous.id_rendezvous;


--
-- Name: urgence; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.urgence (
    id_urgence integer NOT NULL,
    date_incident timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    id_patient integer,
    id_medecin integer
);


ALTER TABLE public.urgence OWNER TO postgres;

--
-- Name: urgence_id_urgence_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.urgence_id_urgence_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.urgence_id_urgence_seq OWNER TO postgres;

--
-- Name: urgence_id_urgence_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.urgence_id_urgence_seq OWNED BY public.urgence.id_urgence;


--
-- Name: users; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.users (
    id integer NOT NULL,
    nom character varying(255),
    prenom character varying(255),
    password character varying(255),
    email character varying(255)
);


ALTER TABLE public.users OWNER TO postgres;

--
-- Name: users_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.users_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.users_id_seq OWNER TO postgres;

--
-- Name: users_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.users_id_seq OWNED BY public.users.id;


--
-- Name: utilisateur; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.utilisateur (
    id_utilisateur integer NOT NULL,
    nom character varying(255),
    prenom character varying(255),
    date_de_naissance date,
    email character varying(255),
    mot_de_passe character varying(255),
    type character varying(50),
    telephone character varying(10),
    id_medecin integer,
    city character varying(255)
);


ALTER TABLE public.utilisateur OWNER TO postgres;

--
-- Name: utilisateur_id_utilisateur_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.utilisateur_id_utilisateur_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.utilisateur_id_utilisateur_seq OWNER TO postgres;

--
-- Name: utilisateur_id_utilisateur_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.utilisateur_id_utilisateur_seq OWNED BY public.utilisateur.id_utilisateur;


--
-- Name: files id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.files ALTER COLUMN id SET DEFAULT nextval('public.files_id_seq'::regclass);


--
-- Name: historique_sante id_historique; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.historique_sante ALTER COLUMN id_historique SET DEFAULT nextval('public.historique_sante_id_historique_seq'::regclass);


--
-- Name: medicament id_medicament; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.medicament ALTER COLUMN id_medicament SET DEFAULT nextval('public.medicament_id_medicament_seq'::regclass);


--
-- Name: rappel id_rappel; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rappel ALTER COLUMN id_rappel SET DEFAULT nextval('public.rappel_id_rappel_seq'::regclass);


--
-- Name: rendez_vous id_rendezvous; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rendez_vous ALTER COLUMN id_rendezvous SET DEFAULT nextval('public.rendez_vous_id_rendezvous_seq'::regclass);


--
-- Name: urgence id_urgence; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.urgence ALTER COLUMN id_urgence SET DEFAULT nextval('public.urgence_id_urgence_seq'::regclass);


--
-- Name: users id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users ALTER COLUMN id SET DEFAULT nextval('public.users_id_seq'::regclass);


--
-- Name: utilisateur id_utilisateur; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateur ALTER COLUMN id_utilisateur SET DEFAULT nextval('public.utilisateur_id_utilisateur_seq'::regclass);


--
-- Data for Name: files; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.files (id, file_name, id_patient) FROM stdin;
1	ord.pdf	10
\.


--
-- Data for Name: historique_sante; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.historique_sante (id_historique, date, type, valeur, id_utilisateur) FROM stdin;
\.


--
-- Data for Name: medicament; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.medicament (id_medicament, nom, description, dosage, id_patient) FROM stdin;
3	doliprane	en cas de douleurs	3 fois	10
5	doliprane	en cas de douleur	3 fois	15
\.


--
-- Data for Name: notification; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.notification (id_rappel, id_medicament) FROM stdin;
\.


--
-- Data for Name: rappel; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.rappel (id_rappel, rendez_vous, conseil, meteo, message, id_patient) FROM stdin;
2	12	\N	\N	Rappel de votre rendez-vous	5
3	16	\N	\N	Rappel de votre rendez-vous	4
5	16	\N	\N	Rappel de votre rendez-vous	4
6	12	\N	\N	Rappel de votre rendez-vous	5
10	26	\N	\N	Rappel de votre rendez-vous	15
15	21	\N	\N	Rappel de votre rendez-vous	4
16	20	\N	\N	Rappel de votre rendez-vous	10
\.


--
-- Data for Name: rendez_vous; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.rendez_vous (id_rendezvous, date, heure, lieu, description, id_patient, id_medecin) FROM stdin;
1	2023-01-15	14:00:00	Clinique Centrale	Consultation annuelle	1	2
2	2023-01-15	14:00:00	Clinique Centrale	Consultation annuelle	1	16
4	2024-01-27	12:00:00	\N	aaaaa	4	\N
5	2024-01-26	05:00:00	\N	azzz	4	0
7	2024-01-27	18:00:00	\N	hello	5	-514319552
8	2024-01-20	18:00:00	\N	aaa	5	517
9	2024-01-24	06:00:00	\N	consu	4	1072191576
10	2024-01-25	06:00:00	\N	azert	4	1
11	2024-01-27	10:00:00	\N	consultation	4	1
12	2024-01-25	20:00:00	2pl du 11 nov	consultation	5	1
13	2024-01-25	10:00:00	21 rue errad	consultation	4	427
14	2024-01-28	07:00:00	21 rue erard	consultation	5	1
15	2024-01-28	16:00:00	20 rue erard	consultation annuelle	7	1072282041
16	2024-01-20	14:00:00	23 rue erard	consultation annuelle	4	1
17	2024-01-26	05:00:00	skjl	consu	7	495
18	2024-01-24	20:00:00	ddd	cons	7	1
19	2024-01-24	10:00:00	dee	dkhjuhede	8	1
20	2024-01-19	10:00:00	rue erard	consultation generale	10	1
21	2024-01-21	09:00:00	dddddd	eeee	4	1
22	2024-01-24	17:00:00	aaaaaaa	aaaaa	5	0
23	2024-01-27	18:00:00	zzzz	aaaaaaaa	5	0
24	2024-01-25	07:00:00	zz	aaaz	5	0
25	2024-01-29	12:00:00	eeeee	zhhhehee	10	0
26	2024-01-20	08:10:00	azzz	a	15	1
27	2024-01-24	06:00:00	aaaaaaaaaa	aaaaaaaa	5	1
28	2024-01-24	06:00:00			5	1
29	2024-01-24	06:00:00	eee	sdd	5	1
30	2024-01-24	06:00:00			5	1
31	2024-01-24	06:00:00	aaaaaaaaaa	aaaaa	5	1
32	2024-01-25	06:05:00	eeee	eeee	4	1
34	2024-01-25	06:10:00	eee	eeee	4	1
35	2024-01-25	06:10:00			4	1
36	2024-01-23	06:00:00	aaaaaaa	aaaaaa	5	1
37	2024-01-23	06:00:00			5	1
38	2024-01-28	11:07:00	zzzzz	zzzzz	4	1
\.


--
-- Data for Name: urgence; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.urgence (id_urgence, date_incident, id_patient, id_medecin) FROM stdin;
\.


--
-- Data for Name: users; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.users (id, nom, prenom, password, email) FROM stdin;
1	chetioui	khaoula	azerty	khaoula@gmail.com
2	chetioui	khaoula	azerty	khaoula@gmail.com
3	chetioui	khaoula	azerty	khaoula@gmail.com
4	chetioui	khaoula	azerty	khaoula@gmail.com
5	chetioui	khaoula	azerty	khaoula@gmail.com
6	chetioui	khaoula	azerty	khaoula@gmail.com
7	chetioui	khaoula	azerty	khaoula@gmail.com
8				
9				
10				
11	chet	khaoula	azerty	khaoulaaa@gmail.fr
12	toto	toto	toto12	toto@gmail;com
\.


--
-- Data for Name: utilisateur; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.utilisateur (id_utilisateur, nom, prenom, date_de_naissance, email, mot_de_passe, type, telephone, id_medecin, city) FROM stdin;
6	khaou	chet	\N	kha@gmail.com	f2d81a260dea8a100dd517984e53c56a7523d96942a834b9cdc249bd4e8c7aa9	Medecin	0987654321	\N	\N
11	yuki	yuki	2024-01-02	yuki@gmail.com	azerty	Patient	0912345678	\N	\N
10	suki	suki	2024-01-11	suki@gmail.com	f2d81a260dea8a100dd517984e53c56a7523d96942a834b9cdc249bd4e8c7aa9	Patient	0987645231	1	\N
12	coco	co	\N	coco@gmail.com	azerty	Medecin	0987563423	\N	\N
14	yuki	yu	2019-01-05	yukii@gmail.com	azerty	Patient	0789654789	\N	\N
15	titi	titi	2024-01-10	titi@gmail.com	azerty	Patient	0978565463	1	\N
5	test	test2	2015-01-30	test2@gmail.com		Patient	1234567890	1	\N
7	gojo	gojo	2007-01-26	gojo	f2d81a260dea8a100dd517984e53c56a7523d96942a834b9cdc249bd4e8c7aa9	Patient	0987654321	\N	\N
8	gojoo	gojo	2024-01-16	gojo@gmail.com	f2d81a260dea8a100dd517984e53c56a7523d96942a834b9cdc249bd4e8c7aa9	Patient	0987654321	\N	\N
1	totoo	toto	\N	toto@gmail.com	azerty	Medecin	0678235621	\N	\N
16	yy	yy	2024-01-10	yy@gmail.com	azerty	Patient	0978654323	\N	\N
4	soso	test	2009-01-24	test@gmail.com		Patient	0987654321	1	Noisy le grand 
2	chet	khaoula	2000-01-01	khaoula@gmail.com		Patient	0765544332	\N	Paris
\.


--
-- Name: files_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.files_id_seq', 1, true);


--
-- Name: historique_sante_id_historique_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.historique_sante_id_historique_seq', 1, false);


--
-- Name: medicament_id_medicament_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.medicament_id_medicament_seq', 5, true);


--
-- Name: rappel_id_rappel_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.rappel_id_rappel_seq', 16, true);


--
-- Name: rendez_vous_id_rendezvous_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.rendez_vous_id_rendezvous_seq', 38, true);


--
-- Name: urgence_id_urgence_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.urgence_id_urgence_seq', 1, true);


--
-- Name: users_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.users_id_seq', 12, true);


--
-- Name: utilisateur_id_utilisateur_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.utilisateur_id_utilisateur_seq', 16, true);


--
-- Name: files files_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.files
    ADD CONSTRAINT files_pkey PRIMARY KEY (id);


--
-- Name: historique_sante historique_sante_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.historique_sante
    ADD CONSTRAINT historique_sante_pkey PRIMARY KEY (id_historique);


--
-- Name: medicament medicament_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.medicament
    ADD CONSTRAINT medicament_pkey PRIMARY KEY (id_medicament);


--
-- Name: notification notification_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.notification
    ADD CONSTRAINT notification_pkey PRIMARY KEY (id_rappel, id_medicament);


--
-- Name: rappel rappel_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rappel
    ADD CONSTRAINT rappel_pkey PRIMARY KEY (id_rappel);


--
-- Name: rendez_vous rendez_vous_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rendez_vous
    ADD CONSTRAINT rendez_vous_pkey PRIMARY KEY (id_rendezvous);


--
-- Name: urgence urgence_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.urgence
    ADD CONSTRAINT urgence_pkey PRIMARY KEY (id_urgence);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);


--
-- Name: utilisateur utilisateur_email_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateur
    ADD CONSTRAINT utilisateur_email_key UNIQUE (email);


--
-- Name: utilisateur utilisateur_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateur
    ADD CONSTRAINT utilisateur_pkey PRIMARY KEY (id_utilisateur);


--
-- Name: files files_id_patient_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.files
    ADD CONSTRAINT files_id_patient_fkey FOREIGN KEY (id_patient) REFERENCES public.utilisateur(id_utilisateur);


--
-- Name: medicament medicament_id_patient_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.medicament
    ADD CONSTRAINT medicament_id_patient_fkey FOREIGN KEY (id_patient) REFERENCES public.utilisateur(id_utilisateur);


--
-- Name: notification notification_id_medicament_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.notification
    ADD CONSTRAINT notification_id_medicament_fkey FOREIGN KEY (id_medicament) REFERENCES public.medicament(id_medicament);


--
-- Name: rappel rappel_id_patient_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rappel
    ADD CONSTRAINT rappel_id_patient_fkey FOREIGN KEY (id_patient) REFERENCES public.utilisateur(id_utilisateur);


--
-- Name: rappel rappel_rendez_vous_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rappel
    ADD CONSTRAINT rappel_rendez_vous_fkey FOREIGN KEY (rendez_vous) REFERENCES public.rendez_vous(id_rendezvous);


--
-- Name: urgence urgence_id_medecin_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.urgence
    ADD CONSTRAINT urgence_id_medecin_fkey FOREIGN KEY (id_medecin) REFERENCES public.utilisateur(id_utilisateur);


--
-- Name: urgence urgence_id_patient_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.urgence
    ADD CONSTRAINT urgence_id_patient_fkey FOREIGN KEY (id_patient) REFERENCES public.utilisateur(id_utilisateur);


--
-- Name: utilisateur utilisateur_id_medecin_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateur
    ADD CONSTRAINT utilisateur_id_medecin_fkey FOREIGN KEY (id_medecin) REFERENCES public.utilisateur(id_utilisateur);


--
-- PostgreSQL database dump complete
--

