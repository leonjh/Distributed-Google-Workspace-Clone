import React, { useEffect, useState } from 'react';
import { ArrowBackIcon } from "@chakra-ui/icons";
import { Button, ButtonGroup, Heading, VStack, Box, Alert, AlertIcon, AlertTitle } from "@chakra-ui/react";
import { Form, Formik } from "formik";
import { useNavigate } from "react-router";
import * as Yup from "yup";
import TextField from "./TextField";

const Register = () => {
    const [error, setError] = useState(false);
    const navigate = useNavigate();

    return (
        <Formik
            initialValues={{ username: "", password: "" }}
            validationSchema={Yup.object({
                username: Yup.string()
                    .required("Username required!")
                    .min(4, "Username too short! Must be at least 5 characters.")
                    .max(28, "Username too long! Must be at most 28 characters."),
                password: Yup.string()
                    .required("Password required!")
                    .min(4, "Password too short! Must be at least 4 characters.")
                    .max(28, "Password too long! Must be at most 28 characters."),
            })}
            onSubmit={(values, actions) => {
                const vals = { ...values };
                actions.resetForm();

                //change to ${window.location.host}
                const url = `http://${window.location.host}/api/account/signup`;
                fetch(url, {
                    method: 'POST',
                    credentials: 'include',
                    headers: {
                        'Accept': 'application/json',
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify(vals),
                }).then(response => {
                    if (!response.ok) {
                        response.json().then((responseJson) => {
                            console.log(responseJson);
                            setError(responseJson.message);
                        });
                    } else {
                        navigate("/home");
                    }
                });
            }}
        >
            <VStack as={Form} w={{ base: "90%", md: "500px" }} m="auto" justify="center" h="100vh" spacing="1rem" >
                <Box p={8} width="500px" maxWidth="5000px" borderWidth={1} borderRadius={8} boxShadow="lg">
                    <Heading justify="center" textAlign={'center'} paddingBottom={2}>Sign Up</Heading>
                    {error.length > 0 && <Alert status='error' mt={3} mb={3}>
                        <AlertIcon />
                        <AlertTitle>{error}</AlertTitle>
                    </Alert>}
                    <TextField name="username" placeholder="Enter username" autoComplete="off" label="Username" />
                    <TextField type="password" name="password" placeholder="Enter password" autoComplete="off" label="Password" />
                    <ButtonGroup pt="1rem">
                        <Button colorScheme="teal" type="submit"> Create Account </Button>
                        <Button onClick={() => navigate("/login")} leftIcon={<ArrowBackIcon />}> Log In </Button>
                    </ButtonGroup>
                </Box>
            </VStack>
        </Formik>
    );
};

export default Register;