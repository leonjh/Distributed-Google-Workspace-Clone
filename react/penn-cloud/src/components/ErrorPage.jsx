import { VStack, Text } from '@chakra-ui/layout';
import { Button } from '@chakra-ui/react';
import React, { useEffect } from 'react';
import { useNavigate } from 'react-router-dom';

const ErrorPage = () => {
  useEffect(() => {
    document.title = "PennCloud - Error 404";
  }, []);

  const navigate = useNavigate();

  return (
    <VStack w={{ base: "90%", md: "500px" }} m="auto" justify="center" h="100vh" spacing="1rem" fontSize="lg">
      <Text fontSize='2xl'>
        Error 404: Page not found
      </Text>
      <Button
        colorScheme="teal"
        bgGradient="linear(to-r, teal.400, teal.500, teal.600)"
        color="white"
        variant="solid"
        onClick={() => navigate("/")}>
        Go Home
      </Button>
    </VStack>
  );
};

export default ErrorPage;