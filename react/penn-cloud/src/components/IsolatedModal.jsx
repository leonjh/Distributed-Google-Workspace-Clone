import {
  Button,
  useDisclosure,
  Modal,
  ModalOverlay,
  ModalContent,
  ModalHeader,
  ModalBody,
  ModalCloseButton,
  TableContainer,
  Table,
  Tr,
  Thead,
  TableCaption,
  Th,
  Tbody,
  Td,
  ModalFooter,
} from '@chakra-ui/react';
import React from 'react';

const IsolatedModal = ({ row_id, values_passed }) => {
  const { isOpen, onOpen, onClose } = useDisclosure();

  var values_json = JSON.stringify(values_passed);

  console.log(values_json);
  return (
    <Td>
      <Button onClick={onOpen} size="sm">
        View Data
      </Button>
      <Modal isOpen={isOpen} onClose={onClose}
        size="full" scrollBehavior='inside'>
        <ModalOverlay />
        <ModalContent>
          <ModalHeader> {row_id} Column data</ModalHeader>
          <ModalCloseButton />
          <ModalBody>
            <TableContainer>
              <Table variant='simple' size="lg">
                <TableCaption>Columns & Corresponding Data</TableCaption>
                <Thead>
                  <Tr>
                    <Th>Column</Th>
                    <Th>Data</Th>
                  </Tr>
                </Thead>
                <Tbody>
                  {Object.entries(values_passed).map(([id_r, values]) => (
                    Object.entries(values).map(([key, value]) => (
                      <Tr key={key}>
                        <Td>
                          {key}
                        </Td>
                        <Td>{value.slice(0, 100)}</Td>
                      </Tr>
                    ))
                  ))}
                </Tbody>
              </Table>
            </TableContainer>
          </ModalBody>
          <ModalFooter>
            <Button colorScheme='blue' mr={3} onClick={onClose}>
              Close
            </Button>
          </ModalFooter>
        </ModalContent>
      </Modal>
    </Td>
  );
};

export default IsolatedModal;